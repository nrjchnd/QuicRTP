from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.backends import default_backend

class SRTPContextMissing(Exception):
    pass

class SRTPContext:
    def __init__(self, key):
        self.key = key
        self.backend = default_backend()
        self.cipher = Cipher(algorithms.AES(self.key), modes.GCM(b'\x00' * 12), backend=self.backend)

    def protect(self, data):
        encryptor = self.cipher.encryptor()
        return encryptor.update(data) + encryptor.finalize()

    def unprotect(self, data):
        decryptor = self.cipher.decryptor()
        return decryptor.update(data) + decryptor.finalize()

def detect_srtp(data, srtp_context):
    try:
        return srtp_context.unprotect(data)
    except Exception:
        raise SRTPContextMissing
