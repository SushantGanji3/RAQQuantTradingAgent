# Deployment Guide

## Production Deployment

### Prerequisites

- Server with Linux/macOS (Ubuntu 20.04+ recommended)
- Docker and Docker Compose (optional)
- API keys for Alpha Vantage and OpenAI
- Minimum 4GB RAM, 2 CPU cores
- 50GB+ disk space for data storage

### Option 1: Docker Deployment (Recommended)

#### 1. Build Docker Image

```bash
docker build -t rag-quant-trading-agent:latest .
```

#### 2. Create Environment File

Create `.env` file:

```env
ALPHA_VANTAGE_API_KEY=your_key
OPENAI_API_KEY=your_key
SERVER_ADDRESS=0.0.0.0:50051
```

#### 3. Run with Docker Compose

```bash
docker-compose up -d
```

#### 4. Verify Deployment

```bash
docker-compose ps
docker-compose logs -f rag-agent
```

### Option 2: Native Deployment

#### 1. Install Dependencies

Follow the build instructions in [BUILD.md](BUILD.md).

#### 2. Build Project

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

#### 3. Create Systemd Service

Create `/etc/systemd/system/rag-agent.service`:

```ini
[Unit]
Description=RAG Quant Trading Agent
After=network.target

[Service]
Type=simple
User=rag-agent
WorkingDirectory=/opt/rag-agent
ExecStart=/opt/rag-agent/build/rag_agent_server
Restart=always
RestartSec=10
Environment="ALPHA_VANTAGE_API_KEY=your_key"
Environment="OPENAI_API_KEY=your_key"

[Install]
WantedBy=multi-user.target
```

#### 4. Enable and Start Service

```bash
sudo systemctl daemon-reload
sudo systemctl enable rag-agent
sudo systemctl start rag-agent
sudo systemctl status rag-agent
```

### Option 3: Kubernetes Deployment

#### 1. Create Kubernetes Deployment

Create `k8s/deployment.yaml`:

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: rag-agent
spec:
  replicas: 3
  selector:
    matchLabels:
      app: rag-agent
  template:
    metadata:
      labels:
        app: rag-agent
    spec:
      containers:
      - name: rag-agent
        image: rag-quant-trading-agent:latest
        ports:
        - containerPort: 50051
        env:
        - name: ALPHA_VANTAGE_API_KEY
          valueFrom:
            secretKeyRef:
              name: rag-agent-secrets
              key: alpha-vantage-api-key
        - name: OPENAI_API_KEY
          valueFrom:
            secretKeyRef:
              name: rag-agent-secrets
              key: openai-api-key
        volumeMounts:
        - name: data
          mountPath: /app/data
        - name: logs
          mountPath: /app/logs
      volumes:
      - name: data
        persistentVolumeClaim:
          claimName: rag-agent-data
      - name: logs
        emptyDir: {}
```

#### 2. Create Service

Create `k8s/service.yaml`:

```yaml
apiVersion: v1
kind: Service
metadata:
  name: rag-agent
spec:
  selector:
    app: rag-agent
  ports:
  - port: 50051
    targetPort: 50051
  type: LoadBalancer
```

#### 3. Deploy

```bash
kubectl apply -f k8s/deployment.yaml
kubectl apply -f k8s/service.yaml
```

### Configuration

#### Environment Variables

- `ALPHA_VANTAGE_API_KEY`: Alpha Vantage API key (required)
- `OPENAI_API_KEY`: OpenAI API key (required)
- `SERVER_ADDRESS`: Server address (default: 0.0.0.0:50051)
- `DB_PATH`: Database path (default: data/trading_data.db)
- `FAISS_INDEX_PATH`: FAISS index path (default: data/faiss_index.index)
- `LOG_LEVEL`: Log level (DEBUG, INFO, WARNING, ERROR)
- `LOG_FILE`: Log file path (default: logs/rag_agent.log)

#### Database Configuration

The SQLite database is stored at `data/trading_data.db`. For production, consider:

1. **PostgreSQL**: More robust for concurrent access
2. **MySQL**: Alternative relational database
3. **DuckDB**: Columnar database for analytics

#### FAISS Index Configuration

For large datasets, consider:

1. **IndexIVFFlat**: Faster search for large indices
2. **GPU Acceleration**: Use FAISS GPU for faster search
3. **Distributed Index**: Split index across multiple servers

### Monitoring

#### Logs

View logs:

```bash
# Docker
docker-compose logs -f rag-agent

# Systemd
sudo journalctl -u rag-agent -f

# Kubernetes
kubectl logs -f deployment/rag-agent
```

#### Metrics

Monitor key metrics:

- Request rate
- Response time
- Error rate
- API call rate
- Database query performance
- FAISS search performance

#### Health Checks

Implement health check endpoint:

```bash
grpc_health_probe -addr=localhost:50051
```

### Scaling

#### Horizontal Scaling

1. Deploy multiple server instances
2. Use load balancer (nginx, HAProxy)
3. Implement session affinity if needed
4. Use distributed cache (Redis)

#### Vertical Scaling

1. Increase CPU cores
2. Increase RAM
3. Use SSD for database
4. Enable GPU for FAISS

### Security

#### API Keys

- Store API keys in environment variables
- Use secrets management (HashiCorp Vault, AWS Secrets Manager)
- Rotate keys regularly
- Use different keys for different environments

#### Network Security

- Use TLS/SSL for gRPC connections
- Implement firewall rules
- Use VPN for internal communications
- Implement rate limiting

#### Authentication

- Implement OAuth 2.0 for API access
- Use API keys for service-to-service communication
- Implement JWT tokens for user authentication

### Backup and Recovery

#### Database Backup

```bash
# Backup SQLite database
sqlite3 data/trading_data.db ".backup backup.db"

# Restore
sqlite3 data/trading_data.db < backup.db
```

#### FAISS Index Backup

```bash
# Backup FAISS index
cp data/faiss_index.index backup/faiss_index.index
cp data/faiss_index.index.meta backup/faiss_index.index.meta
```

#### Automated Backups

Set up cron job:

```bash
0 2 * * * /opt/rag-agent/scripts/backup.sh
```

### Troubleshooting

#### Server Won't Start

1. Check logs: `docker-compose logs rag-agent`
2. Verify API keys are set
3. Check port 50051 is not in use
4. Verify database permissions

#### High Memory Usage

1. Reduce batch size for embeddings
2. Use smaller FAISS index
3. Implement caching
4. Increase server RAM

#### Slow Response Times

1. Optimize database queries
2. Use FAISS IndexIVFFlat for faster search
3. Implement caching (Redis)
4. Scale horizontally

### Maintenance

#### Regular Updates

1. Update dependencies regularly
2. Update API keys if needed
3. Monitor for security vulnerabilities
4. Update FAISS index with new data

#### Data Maintenance

1. Archive old data
2. Clean up unused indices
3. Optimize database
4. Monitor disk space

### Disaster Recovery

#### Backup Strategy

1. Daily database backups
2. Weekly FAISS index backups
3. Store backups in cloud storage
4. Test restore procedures regularly

#### Recovery Procedure

1. Stop server
2. Restore database from backup
3. Restore FAISS index from backup
4. Restart server
5. Verify functionality

### Performance Tuning

#### Database Optimization

```sql
-- Create indexes
CREATE INDEX idx_symbol_timestamp ON ohlcv_data(symbol, timestamp);
CREATE INDEX idx_news_symbol ON news_articles(symbol);
```

#### FAISS Optimization

```cpp
// Use IndexIVFFlat for faster search
faiss::IndexIVFFlat* index = new faiss::IndexIVFFlat(quantizer, dimension_, nlist);
index->train(embeddings);
```

#### Caching

Implement Redis cache for:

- Frequently accessed data
- LLM responses
- Embeddings
- API responses

### Cost Optimization

#### API Costs

1. Cache API responses
2. Batch API requests
3. Use free tier when possible
4. Monitor API usage

#### Infrastructure Costs

1. Use spot instances for non-critical workloads
2. Implement auto-scaling
3. Use reserved instances for predictable workloads
4. Monitor and optimize resource usage

