import redis
import uuid

redis_client = redis.Redis(host='redis', port=6379)

def create_tenant(tenant_name):
    tenant_id = str(uuid.uuid4())
    api_key = str(uuid.uuid4())
    redis_client.hmset(f"tenant:{tenant_id}", {"name": tenant_name, "api_key": api_key, "usage": 0})
    return tenant_id, api_key

def get_tenant_by_api_key(api_key):
    keys = redis_client.keys("tenant:*")
    for key in keys:
        tenant = redis_client.hgetall(key)
        if tenant[b'api_key'].decode('utf-8') == api_key:
            return key.decode('utf-8'), tenant
    return None, None

def update_usage(tenant_id, usage):
    redis_client.hincrby(f"tenant:{tenant_id}", "usage", usage)

def get_billing_info(tenant_id):
    tenant = redis_client.hgetall(f"tenant:{tenant_id}")
    name = tenant[b'name'].decode('utf-8')
    usage = int(tenant[b'usage'].decode('utf-8'))
    return {"name": name, "usage": usage}

def get_all_tenants():
    keys = redis_client.keys("tenant:*")
    tenants = []
    for key in keys:
        tenant = redis_client.hgetall(key)
        tenants.append({
            "id": key.decode('utf-8'),
            "name": tenant[b'name'].decode('utf-8'),
            "api_key": tenant[b'api_key'].decode('utf-8'),
            "usage": int(tenant[b'usage'].decode('utf-8'))
        })
    return tenants

def store_proxy_config(tenant_id, config):
    redis_client.hmset(f"config:{tenant_id}", config)

def get_proxy_config(tenant_id):
    config = redis_client.hgetall(f"config:{tenant_id}")
    return {key.decode('utf-8'): value.decode('utf-8') for key, value in config.items()}
