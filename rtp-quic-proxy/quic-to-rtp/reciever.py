import asyncio
import logging
import redis
from aioquic.asyncio import serve
from aioquic.asyncio.protocol import QuicConnectionProtocol
from aioquic.quic.configuration import QuicConfiguration
from aioquic.quic.events import StreamDataReceived
from billing import track_bandwidth
from srtp import SRTPContext, detect_srtp, SRTPContextMissing

logging.basicConfig(level=logging.INFO)

class QUICToRTPProxy(QuicConnectionProtocol):
    def __init__(self, *args, srtp_key, tenant_id, **kwargs):
        super().__init__(*args, **kwargs)
        self.srtp_context = SRTPContext(srtp_key)
        self.tenant_id = tenant_id

    def quic_event_received(self, event):
        if isinstance(event, StreamDataReceived):
            self.handle_stream_data(event.stream_id, event.data)

    def handle_stream_data(self, stream_id, data):
        try:
            rtp_data = self.srtp_context.unprotect(data)
        except SRTPContextMissing:
            rtp_data = data

        track_bandwidth(self.tenant_id, len(data))

async def main():
    redis_client = redis.Redis(host='redis', port=6379)
    quic_host = redis_client.get('receiver_host').decode('utf-8')
    quic_port = int(redis_client.get('receiver_port').decode('utf-8'))

    srtp_key = b'\x00' * 30
    tenant_id = redis_client.get('tenant_id').decode('utf-8')

    configuration = QuicConfiguration(is_client=False)

    def create_protocol(*args, **kwargs):
        return QUICToRTPProxy(*args, srtp_key=srtp_key, tenant_id=tenant_id, **kwargs)

    await serve(quic_host, quic_port, configuration=configuration, create_protocol=create_protocol)

if __name__ == '__main__':
    asyncio.run(main())
