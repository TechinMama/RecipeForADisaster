# RecipeForADisaster Production Deployment Guide

This guide covers deploying RecipeForADisaster in a production environment using Docker and Docker Compose.

## Prerequisites

- Docker and Docker Compose installed
- At least 2GB RAM available
- 5GB free disk space
- Linux/macOS/Windows with WSL2

## Quick Start

1. **Clone and setup:**
   ```bash
   git clone <repository-url>
   cd RecipeForADisaster
   ```

2. **Configure environment:**
   ```bash
   cp .env.example .env
   # Edit .env with your production values
   nano .env
   ```

3. **Deploy:**
   ```bash
   ./deploy-production.sh
   ```

## Environment Configuration

### Critical Security Settings

Edit `.env` and update these values:

```bash
# Generate a secure JWT secret (required)
JWT_SECRET=your-super-secure-random-secret-here

# Database paths (recommended for production)
RECIPES_DB_PATH=/app/data/recipes.db
USERS_DB_PATH=/app/data/users.db

# Azure OpenAI (optional, for AI features)
AZURE_OPENAI_ENDPOINT=https://your-resource.openai.azure.com/
AZURE_OPENAI_KEY=your-secure-api-key
AZURE_OPENAI_DEPLOYMENT=gpt-35-turbo
```

### JWT Secret Generation

Generate a secure JWT secret:
```bash
openssl rand -hex 32
```

## Production Architecture

```
Internet → Nginx (80/443) → RecipeForADisaster App (8080)
                              ↓
                         SQLite Databases
                         (data/ directory)
```

### Services

- **App**: Main application server (C++/Crow)
- **Nginx**: Reverse proxy and load balancer (optional, production profile)

## Deployment Options

### Basic Deployment (API only)
```bash
docker-compose up -d app
```

### Full Production Deployment (with Nginx)
```bash
docker-compose --profile production up -d
```

## SSL/HTTPS Configuration

1. **Obtain SSL certificates** (Let's Encrypt recommended)
2. **Place certificates** in `nginx/ssl/`:
   ```
   nginx/ssl/cert.pem  # Certificate
   nginx/ssl/key.pem   # Private key
   ```
3. **Uncomment SSL server block** in `nginx/nginx.conf`
4. **Restart services**:
   ```bash
   docker-compose --profile production up -d
   ```

## Data Persistence

- **Database files**: Stored in `data/` directory (Docker volume)
- **Logs**: Stored in `logs/` directory
- **Backup strategy**: Regularly backup `data/` directory

## Monitoring

### Health Checks
```bash
curl http://localhost/api/health
```

### View Logs
```bash
# All services
docker-compose logs -f

# Specific service
docker-compose logs -f app
docker-compose logs -f nginx
```

### Service Status
```bash
docker-compose ps
```

## Security Considerations

### Network Security
- Configure firewall to only allow necessary ports (80, 443, 22)
- Use HTTPS in production
- Consider using a VPN for database access

### Application Security
- Change default JWT secret
- Use strong passwords for all accounts
- Regularly update Docker images
- Monitor logs for suspicious activity

### Database Security
- SQLite files are world-readable in container
- Consider using a proper database (PostgreSQL/MySQL) for high-traffic sites
- Implement database encryption if sensitive data is stored

## Backup and Recovery

### Database Backup
```bash
# Stop services before backup
docker-compose down

# Backup data directory
tar -czf backup-$(date +%Y%m%d).tar.gz data/

# Restart services
docker-compose up -d
```

### Restore from Backup
```bash
# Stop services
docker-compose down

# Restore data
tar -xzf backup-20231201.tar.gz

# Start services
docker-compose up -d
```

## Troubleshooting

### Common Issues

1. **Port 8080 already in use**
   ```bash
   # Find process using port
   lsof -i :8080
   # Kill process or change port in docker-compose.yml
   ```

2. **Permission denied on data directory**
   ```bash
   sudo chown -R $USER:$USER data/
   ```

3. **Database corruption**
   ```bash
   # Stop services
   docker-compose down
   # Remove corrupted database
   rm data/*.db
   # Restart (will recreate database)
   docker-compose up -d
   ```

### Performance Tuning

- **Memory**: Increase Docker memory limit if needed
- **CPU**: Application is single-threaded, consider multiple instances with load balancer
- **Storage**: Monitor disk space usage in `data/` directory

## Updating Deployment

```bash
# Pull latest changes
git pull

# Rebuild and restart
docker-compose up -d --build
```

## Production Checklist

- [ ] Environment variables configured
- [ ] JWT secret changed from default
- [ ] SSL certificates installed
- [ ] Firewall configured
- [ ] Backup strategy implemented
- [ ] Monitoring set up
- [ ] Regular security updates scheduled

## Support

For issues and questions:
1. Check logs: `docker-compose logs`
2. Verify configuration in `.env`
3. Test API endpoints manually
4. Check system resources (RAM, disk space)