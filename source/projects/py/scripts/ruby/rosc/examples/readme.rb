require 'osc'
Host = 'localhost'
Port = 5000

s = OSC::UDPServer.new
s.bind Host, Port

c = OSC::UDPSocket.new
m = OSC::Message.new('/foo', 'fi', Math::PI, 42)
c.send m, 0, Host, Port

s.add_method '/f*', 'fi' do |msg|
  domain, port, host, ip = msg.source
  puts "#{msg.address} -> #{msg.args.inspect} from #{host}:#{port}"
end
Thread.new do
  s.serve
end
sleep 5
