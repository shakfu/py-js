require 'time'
require 'forwardable'
require 'stringio'
require 'yaml'

# Test for broken pack/unpack
if [1].pack('n') == "\001\000"
  class String
    alias_method :broken_unpack, :unpack
    def unpack(spec)
      broken_unpack(spec.tr("nNvV","vVnN"))
    end
  end
  class Array
    alias_method :broken_pack, :pack
    def pack(spec)
      broken_pack(spec.tr("nNvV","vVnN"))
    end
  end
end


class StringIO
  def skip(n)
    self.seek(n, IO::SEEK_CUR)
  end
  def skip_padding
    self.skip((4-pos)%4)
  end
end

# Of particular interest are OSC::Client, OSC::Server, OSC::Message and
# OSC::Bundle.
module OSC
  # 64-bit big-endian fixed-point time tag
  class TimeTag
    JAN_1970 = 0x83aa7e80
    # nil:: immediately
    # Numeric:: seconds since January 1, 1900 00:00
    # Numeric,Numeric:: int,frac parts of a TimeTag.
    # Time:: convert from Time object
    def initialize(*args)
      t = args
      t = t.first if t and t.size == 1
      case t
      when NIL # immediately
        @int = 0
        @frac = 1
      when Numeric
        @int, fr = t.divmod(1)
        @frac = (fr * (2**32)).to_i
      when Array
        @int,@frac = t
      when Time
        @int, fr = (t.to_f+JAN_1970).divmod(1)
        @frac = (fr * (2**32)).to_i
      else
        raise ArgumentError
      end
    end
    attr_accessor :int, :frac
    def to_i; to_f.to_i; end
    # Ruby's Float can handle the 64 bits so we have the luxury of dealing with
    # Float directly
    def to_f; @int.to_f + @frac.to_f/(2**32); end
    # [int,frac]
    def to_a; [@int,@frac]; end
    # Human-readable, like the output of Time#to_s
    def to_s; to_time.to_s; end
    # Ruby Time object
    def to_time; Time.at(to_f-JAN_1970); end
    alias :time :to_time
    def self.now; TimeTag.new(Time.now); end
    def method_missing(sym, *args)
      time.__send__(sym, *args)
    end
    def to_yaml
      to_a.to_yaml
    end
  end

  class Blob < String
  end

  class Message
    attr_accessor :address, :args
    # The source of this message, usually something like ["AF_INET", 50475,
    # 'localhost','127.0.0.1']
    attr_accessor :source

    # address:: The OSC address (a String)
    # types:: The OSC type tags string
    # args:: arguments. must match type tags in arity
    #
    # Example:
    #   Message.new('/foo','ff', Math::PI, Math::E)
    #
    # Arguments will be coerced as indicated by the type tags. If types is nil,
    # type tags will be inferred from arguments.
    def initialize(address, types=nil, *args)
      if types and types.size != args.size
        raise ArgumentError, 'type/args arity mismatch'
      end

      @address = address
      @args = []

      if types
        args.each_with_index do |arg, i|
          case types[i]
          when ?i; @args << arg.to_i
          when ?f; @args << arg.to_f
          when ?s; @args << arg.to_s
          when ?b; @args << Blob.new(arg)
          else
            raise ArgumentError, "unknown type tag '#{@types[i].inspect}'"
          end
        end
      else
        args.each do |arg|
          case arg
          when Fixnum,Float,String,TimeTag,Blob
            @args << arg
          else
            raise ArgumentError, "Object has unknown OSC type: '#{arg}'"
          end
        end
      end
    end

    def types
      @args.collect {|a| Packet.tag a}.join
    end
    alias :typetag :types

    # Encode this message for transport
    def encode
      Packet.encode(self)
    end
    # string representation. *not* the raw representation, for that use
    # encode.
    def to_s
      "#{address},#{types},#{args.collect{|a| a.to_s}.join(',')}"
    end
    def to_yaml
      {'address'=>address, 'types'=>types, 'args'=>args}.to_yaml
    end

    extend Forwardable
    include Enumerable

    de = (Array.instance_methods - self.instance_methods)
    de -= %w(assoc flatten flatten! pack rassoc transpose)
    de += %w(include? sort)

    def_delegators(:@args, *de)

    undef_method :zip
  end

  # bundle of messages and/or bundles
  class Bundle
    attr_accessor :timetag
    attr_accessor :args
    attr_accessor :source
    alias :timestamp :timetag
    alias :messages :args
    alias :contents :args
    alias :to_a :args

    # New bundle with time and messages
    def initialize(t=nil, *args)
      @timetag =
        case t
        when TimeTag
          t
        else
          TimeTag.new(t)
        end
      @args = args
    end

    def to_yaml
      {'timestamp'=>timetag, 'contents'=>contents}.to_yaml
    end

    extend Forwardable
    include Enumerable

    de = (Array.instance_methods - self.instance_methods)
    de -= %w(assoc flatten flatten! pack rassoc transpose)
    de += %w(include? sort)

    def_delegators(:@args, *de)

    undef_method :zip

    def encode
      Packet.encode(self)
    end

  end

  # Unit of transmission.  Really needs revamping
  module Packet
    # XXX I might fold this and its siblings back into the decode case
    # statement
    def self.decode_int32(io)
      i = io.read(4).unpack('N')[0]
      i = 2**32 - i if i > (2**31-1) # two's complement
      i
    end

    def self.decode_float32(io)
      f = io.read(4).unpack('g')[0]
      f
    end

    def self.decode_string(io)
      s = io.gets("\0").chomp("\0")
      io.skip_padding
      s
    end

    def self.decode_blob(io)
      l = io.read(4).unpack('N')[0]
      b = io.read(l)
      io.skip_padding
      b
    end

    def self.decode_timetag(io)
      t1 = io.read(4).unpack('N')[0]
      t2 = io.read(4).unpack('N')[0]
      TimeTag.new [t1,t2]
    end

    # Takes a string containing one packet
    def self.decode(packet)
      # XXX I think it would have been better to use a StringScanner. Maybe I
      # will convert it someday...
      if (packet == nil)
        return Message.new("/")
      end
      io = StringIO.new(packet)
      id = decode_string(io)
      if id == '#bundle'
        b = Bundle.new(decode_timetag(io))
        until io.eof?
          l = io.read(4).unpack('N')[0]
          s = io.read(l)
          b << decode(s)
        end
        b
      elsif id =~ /^\//
        m = Message.new(id)
        if io.getc == ?,
          tags = decode_string(io)
          tags.scan(/./) do |t|
            case t
            when 'i'
              m << decode_int32(io)
            when 'f'
              m << decode_float32(io)
            when 's'
              m << decode_string(io)
            when 'b'
              m << decode_blob(io)

              # right now we skip over nonstandard datatypes, but we'll want to
              # add these datatypes too.
            when /[htd]/; io.read(8)
            when 'S'; decode_string(io)
            when /[crm]/; io.read(4)
            when /[TFNI\[\]]/;
            end
          end
        end
        m
      end
    end

    def self.pad(s)
      s + ("\000" * ((4 - s.size)%4))
    end

    def self.tag(o)
      case o
      when Fixnum;  'i'
      when TimeTag; 't'
      when Float;   'f'
      when Blob;    'b'
      when String;  's'
      else;         nil
      end
    end

    def self.encode(o)
      case o
      when Fixnum;  [o].pack 'N'
      when Float;   [o].pack 'g'
      when Blob;    pad([o.size].pack('N') + o)
      when String;  pad(o.sub(/\000.*\Z/, '') + "\000")
      when TimeTag; o.to_a.pack('NN')

      when Message
        s = encode(o.address)
        s << encode(','+o.types)
        s << o.args.collect{|x| encode(x)}.join

      when Bundle
        s = encode('#bundle')
        s << encode(o.timetag)
        s << o.args.collect { |x|
          x2 = encode(x); [x2.size].pack('N') + x2
        }.join
      end
    end

    private_class_method :decode_int32, :decode_float32, :decode_string,
    :decode_blob, :decode_timetag
  end
end


libdir = Dir.getwd + "/rosc/lib"
olddir = Dir.getwd
Dir.chdir libdir        # change to libdir so that requires work
require "#{libdir}/osc/pattern"
require "#{libdir}/osc/server"
require "#{libdir}/osc/udp"
require "#{libdir}/osc/udp_server_with_count"
Dir.chdir olddir

