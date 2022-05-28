module OSC
  # Mixin for OSC transports. You implement (or in many cases just alias)
  # send_raw, recvfrom_raw, and recv_raw, which have the semantics of send,
  # recvfrom, and recv in e.g. UDPSocket
  module Transport
    # Send a Message, Bundle, or even raw data
    def send(msg, *args)
      case msg
      when Message,Bundle
        send_raw(msg.encode, *args)
      else
        send_raw(msg, *args)
      end
    end

    # Receive a Message, Bundle, or raw data and the sender. The source
    # attribute of the Message or Bundle is also set to sender. e.g.
    #   packet, sender = udp_osc_client.recvfrom(32768)
    def recvfrom(*args)
      data, sender = recvfrom_raw(*args)
      m = Packet.decode(data)
      m.source = sender
      [m, sender]
    rescue
      [data, sender]
    end

    # Receive a Message, Bundle, or raw data.
    def recv(*args)
      data = recv_raw(*args)
      Packet.decode(data)
    rescue
    end

    # Send a Message/Bundle with a timestamp (a Time or TimeTag object).
    def send_timestamped(msg, ts, *args)
      m = Bundle.new(ts, msg)
      send(m, *args)
    end
    alias :send_ts :send_timestamped
  end
end
