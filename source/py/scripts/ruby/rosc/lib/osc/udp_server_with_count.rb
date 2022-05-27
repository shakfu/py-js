module OSC
  class UDPServerWithCount < UDPServer
    attr_reader :num_messages_received

    def initialize
      @num_messages_received = 0
      super
    end

    def serve
      #only loop when we're receiving non-nil values
      continue = true
      while continue do
        p, sender = recvfrom(MAX_MSG_SIZE)
        if p
          dispatch p
          @num_messages_received += 1
        else
          continue = false
        end
      end
    end
  end
end
