require "#{Dir.pwd}/osc/transport"
require "socket"

module OSC
  # A ::UDPSocket with a send method that accepts a Message or Bundle or
  # a raw String.
  class UDPSocket < ::UDPSocket
    alias :send_raw :send
    alias :recvfrom_raw :recvfrom
    alias :recv_raw :recv
    include Transport
  end

  class UDPServer < OSC::UDPSocket
    MAX_MSG_SIZE=32768
    include Server
    def serve
      loop do
        p, sender = recvfrom(MAX_MSG_SIZE)
        dispatch p
      end
    end

    # send msg2 as a reply to msg1
    def reply(msg1, msg2)
      domain, port, host, ip = msg2.source
      send(msg2, 0, host, port)
    end
  end
end
