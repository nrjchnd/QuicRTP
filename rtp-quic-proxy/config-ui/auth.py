import jwt
from functools import wraps
from flask import request, jsonify
from models import get_tenant_by_api_key

SECRET_KEY = 'YOUR_SECRET_KEY'

def generate_token(username):
    return jwt.encode({'username': username}, SECRET_KEY, algorithm='HS256')

def verify_token(token):
    try:
        decoded = jwt.decode(token, SECRET_KEY, algorithms=['HS256'])
        return decoded['username']
    except jwt.ExpiredSignatureError:
        return None

def login_required(f):
    @wraps(f)
    def decorated_function(*args, **kwargs):
        token = request.headers.get('Authorization')
        if not token:
            return jsonify({'message': 'Token is missing!'}), 401
        try:
            token = token.split(" ")[1]
            current_user = verify_token(token)
            if not current_user:
                return jsonify({'message': 'Invalid token!'}), 401
        except IndexError:
            return jsonify({'message': 'Token is missing!'}), 401
        return f(current_user, *args, **kwargs)
    return decorated_function

def api_key_required(f):
    @wraps(f)
    def decorated_function(*args, **kwargs):
        api_key = request.headers.get('X-API-Key')
        if not api_key:
            return jsonify({'message': 'API key is missing!'}), 401
        tenant_id, tenant = get_tenant_by_api_key(api_key)
        if not tenant_id:
            return jsonify({'message': 'Invalid API key!'}), 401
        return f(tenant_id, *args, **kwargs)
    return decorated_function
