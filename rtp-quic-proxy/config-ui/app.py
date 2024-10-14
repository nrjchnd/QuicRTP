from flask import Flask, request, jsonify
from auth import generate_token, login_required, api_key_required
from models import create_tenant, get_tenant_by_api_key, update_usage, get_billing_info, get_all_tenants, store_proxy_config, get_proxy_config

app = Flask(__name__)

# Admin Login Endpoint
@app.route('/login', methods=['POST'])
def login():
    data = request.json
    if data['username'] == 'admin' and data['password'] == 'admin':
        token = generate_token('admin')
        return jsonify({'token': token}), 200
    return jsonify({'message': 'Invalid credentials'}), 401

# Tenant Management Endpoints
@app.route('/tenants', methods=['POST'])
@login_required
def create_new_tenant(current_user):
    data = request.json
    tenant_id, api_key = create_tenant(data['name'])
    return jsonify({'tenant_id': tenant_id, 'api_key': api_key}), 201

@app.route('/tenants', methods=['GET'])
@login_required
def list_tenants(current_user):
    tenants = get_all_tenants()
    return jsonify(tenants), 200

@app.route('/tenants/<tenant_id>/billing', methods=['GET'])
@login_required
def get_billing(current_user, tenant_id):
    billing_info = get_billing_info(tenant_id)
    return jsonify(billing_info), 200

# Proxy Configuration Endpoints
@app.route('/config', methods=['POST'])
@api_key_required
def store_configuration(tenant_id):
    data = request.json
    store_proxy_config(tenant_id, data)
    return jsonify({'status': 'success'}), 200

@app.route('/config', methods=['GET'])
@api_key_required
def get_configuration(tenant_id):
    config = get_proxy_config(tenant_id)
    return jsonify(config), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080)
