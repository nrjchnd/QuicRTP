# RTP to QUIC Bridge Proxy

This project provides a multi-tenant RTP-to-QUIC bridge proxy service. The system accepts incoming RTP/SRTP streams, multiplexes and converts them to QUIC transport, and sends them securely to a corresponding proxy that reverses the process.

## Features
- **RTP/SRTP Detection:** Automatically detect and handle both RTP and SRTP streams.
- **Multi-Tenant Access Control:** Allows multiple tenants to use the bridge service securely.
- **Billing Model:** Track bandwidth usage per tenant and provide billing information.
- **Configuration Web UI:** Simple web-based configuration interface for creating and managing tenants.

