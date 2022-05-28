$:.unshift(File.dirname(__FILE__) + "/../lib/")
require 'osc'
require 'time'
require 'test/unit'

class TC_OSC < Test::Unit::TestCase
  include OSC
  # def setup
  # end

  # def teardown
  # end

  def test_datatype
    s = 'foo'
    i = 42
    f = 3.14

    assert_equal 'i', Packet.tag(i)
    assert_equal 'f', Packet.tag(f)
    assert_equal 's', Packet.tag(s)
    assert_equal s+"\000", Packet.encode(s)
    b = Blob.new("foobardoobar\0\0x200")
    assert_equal 'b', Packet.tag(b)
    assert_equal b.size+4 + (b.size+4)%4, Packet.encode(b).size
  end

  def test_timetag
    t1 = TimeTag::JAN_1970
    t2 = Time.now
    t3 = t2.to_f+t1

    tt = TimeTag.new t2
    assert_equal t3, tt.to_f
    assert_equal t3.floor, tt.to_i
    assert_equal t3.floor - t3, tt.to_i - tt.to_f
    assert_equal [0,1].pack('NN'), Packet.encode(TimeTag.new(nil))
    assert_equal t2.to_i,tt.to_time.to_i # to_f has roundoff error at the lsb
  end

  def test_message
    a = 'foo'
    b = 'quux'
    m = Message.new '/foobar', 'ssi', a, b, 1
    assert_equal "/foobar\000"+",ssi\000\000\000\000"+
      "foo\000"+"quux\000\000\000\000"+"\000\000\000\001", Packet.encode(m)
  end

  def test_bundle
    m1 = Message.new '/foo','s','foo'
    m2 = Message.new '/bar','s','bar'
    t = Time.now
    b = Bundle.new(TimeTag.new(Time.at(t + 10)), m1, m2)
    b2 = Bundle.new(nil, b, m1)

    assert_equal 10, b.timetag.to_time.to_i - t.to_i
    e = Packet.encode(b2)
    assert_equal '#bundle', e[0,7]
    assert_equal "\000\000\000\000\000\000\000\001", e[8,8]
    assert_equal '#bundle', e[16+4,7]
    assert_equal '/foo', e[16+4+Packet.encode(b).size+4,4]
    assert_equal 0, e.size % 4

    assert_instance_of Array, b2.to_a
    assert_instance_of Bundle, b2.to_a[0]
    assert_instance_of Message, b2.to_a[1]

    bundle = Packet.decode(e)
    assert_instance_of Bundle, bundle


  end



  def test_packet
    m = Message.new '/foo','s','foo'
    b = Bundle.new nil,m

    m2 = Packet.decode("/foo\000\000\000\000,s\000\000foo\000")
    assert_equal m.address,m2.address
    m2 = Packet.decode(Packet.encode(m))
    assert_equal m.address,m2.address
    assert_equal m.typetag,m2.typetag
    assert_equal m.args.size,m2.args.size
    b2 = Packet.decode(Packet.encode(b))
    assert_equal b.args.size,b2.args.size
  end

  class TestServer
    include OSC::Transport
    include OSC::Server
    def test_request(p)
      send p
    end

    def send_raw(msg, *args)
      dispatch msg
    end

  end

  def test_server
    s = TestServer.new
    s.add_method('/foo/bar',nil) { |msg|
      assert_equal 'si',msg.typetag
      assert_equal 'Hello, World!',msg[0]
      assert_equal 42,msg[1]
    }
    s.test_request Message.new("/foo/bar",'si','Hello, World!',42)
  end

  def test_server_with_bundle
    s = TestServer.new
    s.add_method('/foo/bar',nil) { |msg|
      assert_equal 'si',msg.typetag
      assert_equal 'Hello, World!',msg[0]
      assert_equal 42,msg[1]
    }
    s.test_request Bundle.new(nil, Message.new("/foo/bar",'si','Hello, World!',42), Message.new("/foo/bar",'si','Hello, World!',42), Message.new("/foo/bar",'si','Hello, World!',42))
  end

  def test_pattern
    # test *
    assert Pattern.intersect?('/*/bar/baz','/foo/*/baz')
    assert Pattern.intersect?('/f*','/*o')
    assert ! Pattern.intersect?('/f*','/foo/bar')
    assert ! Pattern.intersect?('/f*','/bar')
    # test ?
    assert Pattern.intersect?('/fo?/bar','/foo/?ar')
    assert ! Pattern.intersect?('/foo?','/foo')
    # test []
    assert Pattern.intersect?('/foo/ba[rz]','/foo/bar')
    assert Pattern.intersect?('/[!abcde]/a','/[!abcde]/a')
    assert Pattern.intersect?('/[!abcde]/a','/f/a')
    assert Pattern.intersect?('/[!abcde]/a','/[abf]/a')
    assert ! Pattern.intersect?('/[ab]/a','/[!abc]/a')
    assert ! Pattern.intersect?('/[abcde]','/[!abcde]')
    assert ! Pattern.intersect?('/[abcde]','/f')
    assert ! Pattern.intersect?('/[!abcde]','/a')
    # test {}
    assert Pattern.intersect?('/{foo,bar,baz}','/foo')
    assert Pattern.intersect?('/{foo,bar,baz}','/bar')
    assert Pattern.intersect?('/{foo,bar,baz}','/baz')
    assert ! Pattern.intersect?('/{foo,bar,baz}','/quux')
    assert ! Pattern.intersect?('/{foo,bar,baz}','/fo')
    # * with *,?,[]
    assert Pattern.intersect?('/*/bar','/*/ba?')
    assert Pattern.intersect?('/*/bar','/*x/ba?')
    assert Pattern.intersect?('/*/bar','/?/ba?')
    assert Pattern.intersect?('/*/bar','/?x/ba?')
    assert Pattern.intersect?('/*/bar','/[abcde]/ba?')
    assert Pattern.intersect?('/*/bar','/[abcde]x/ba?')
    assert Pattern.intersect?('/*/bar','/[!abcde]/ba?')
    assert Pattern.intersect?('/*/bar','/[!abcde]x/ba?')
    # ? with []
    assert Pattern.intersect?('/?','/[abcde]')
    assert Pattern.intersect?('/?','/[!abcde]')
  end
end
