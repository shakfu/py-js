module OSC
  # Mixin for making servers.
  # Your job is to read a packet and call dispatch(Packet.decode(raw)), ad
  # infinitum (e.g. in a method named serve).
  module Server
    #   prock.respond_to?(:call) #=> true
    # Pass an OSC pattern, a typespec, and either prock or a block.
    # The block/prock will be called if the pattern and typspec match. Numeric
    # types will be coerced, so e.g. 'fi' would match 'ii' and the float would
    # be coerced to an int.
    def add_method(pat, typespec, prock=nil, &block)
      pat = Pattern.new(pat) unless (Pattern === pat || pat.nil?)
      if block_given? and prock
        raise ArgumentError, 'Specify either a block or a Proc, not both.'
      end
      prock = block if block_given?
      unless prock.respond_to?(:call)
        raise ArgumentError, "Prock doesn't respond to :call"
      end
      unless typespec.nil? or typespec =~ /[ifsb]*/
        raise ArgumentError, "Bad typespec '#{typespec}'"
      end
      @cb ||= []
      @cb << [pat, typespec, prock]
    end

    # dispatch the provided message. It can be raw or already decoded with
    # Packet.decode
    def dispatch(mesg)
      case mesg
      when Bundle, Message
      else
        mesg = Packet.decode(mesg)
      end

      case mesg
      when Bundle; dispatch_bundle(mesg)
      when Message
        unless @cb.nil?
          @cb.each do |pat, typespec, obj|
            if pat.nil? or Pattern.intersect?(pat, mesg.address)
              if typespec
                if typespec.size == mesg.args.size
                  match = true
                  typespec.size.times do |i|
                    c = typespec[i]
                    case c
                    when ?i, ?f
                      match &&= (Numeric === mesg.args[i])
                    when ?s, ?b
                      match &&= (String === mesg.args[i])
                    end
                  end
                  if match
                    typespec.size.times do |i|
                      case typespec[i]
                      when ?i
                        mesg.args[i] = mesg.args[i].to_i
                      when ?f
                        mesg.args[i] = mesg.args[i].to_f
                      when ?s,?b
                        mesg.args[i] = mesg.args[i].to_s
                        mesg.args[i] = mesg.args[i].to_s
                      end
                    end
                    obj.call(mesg)
                  end
                end
              else # no typespec
                obj.call(mesg)
              end
            end # pattern match
          end # @cb.each
        end # unless @cb.nil?
      else
        raise "bad mesg"
      end
    end

    # May create a new thread to wait to dispatch according to p.timetag.
    def dispatch_bundle(p)
      diff = p.timetag.to_f - TimeTag.now.to_f
      if diff <= 0
        p.each {|m| m.source = p.source; dispatch m}
      else
        Thread.new do
          sleep diff
          p.each {|m| m.source = p.source; dispatch m}
        end
      end
    end
  end
end
