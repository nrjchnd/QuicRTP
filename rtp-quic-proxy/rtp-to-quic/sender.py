import asyncio
import logging
import redis
from aioquic.asyncio import connect
from aioquic.quic.configuration import QuicConfiguration
from billing import track_bandwidth
from srtp import SRTPContext, detect_srtp, SRTPContextMissing

logging.basicConfig(level=logging.INFO)

class RTPToQUICProxy:
    def __init__(self, quic_host, quic_port, rtp_ports, srtp_key, tenant_id):
        self.quic_host = quic_host
        self.quic_port = quic_port
        self.rtp_ports = rtp_ports
        self.srtp_key = srtp_key
        self.tenant_id = tenant_id
        self.srtp_context = SRTPContext(srtp_key)

    async def start(self):
        configuration = QuicConfiguration(is_client=True)
        self.quic_protocol = await connect(self.quic_host, self.quic_port, configuration=configuration)
        for port in self.rtp_ports:
            asyncio.create_task(self.start_rtp_listener(port))

    async def start_rtp_listener(self, port):
        logging.info(f"Listening for RTP/SRTP on port {port}")
        await asyncio.get_event_loop().create_datagram_endpoint(
            lambda: RTPHandler(self.quic_protocol, self.srtp_context, self.tenant_id),
            local_addr=("0.0.0.0", port)
        )

class RTPHandler(asyncio.DatagramProtocol):
    def __init__(self, quic_protocol, srtp_context, tenant_id):
        self.quic_protocol = quic_protocol
        self.srtp_context = srtp_context
        self.tenant_id = tenant_id

    def datagram_received(self, data, addr):
        try:
            encrypted_data = detect_srtp(data, self.srtp_context)
        except SRTPContextMissing:
            encrypted_data = data

        self.quic_protocol.send_stream_data(0, encrypted_data)
        track_bandwidth(self.tenant_id, len(encrypted_data))

async def main():
    redis_client = redis.Redis(host='redis', port=6379)
    quic_host = redis_client.get('receiver_host').decode('utf-8')
    quic_port = int(redis_client.get('receiver_port').decode('utf-8'))
    rtp_ports = list(map(int, redis_client.get('sender_ports').decode('utf-8').split(',')))

    srtp_key =
