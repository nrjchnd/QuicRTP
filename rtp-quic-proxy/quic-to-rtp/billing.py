import redis

redis_client = redis.Redis(host='redis', port=6379)

def track_bandwidth(tenant_id, bytes_transferred):
    redis_client.hincrby(f"tenant:{tenant_id}", "usage", bytes_transferred)
