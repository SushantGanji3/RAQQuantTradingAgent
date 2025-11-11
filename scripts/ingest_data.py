#!/usr/bin/env python3
"""
Data ingestion script to fetch and store market data using Alpha Vantage API.
"""

import os
import sys
import sqlite3
import requests
import json
import argparse
from datetime import datetime, timedelta
from typing import List, Dict, Optional

# Load environment variables
def load_env():
    """Load environment variables from .env file."""
    env_file = os.path.join(os.path.dirname(__file__), '..', '.env')
    if os.path.exists(env_file):
        with open(env_file, 'r') as f:
            for line in f:
                line = line.strip()
                if line and not line.startswith('#') and '=' in line:
                    key, value = line.split('=', 1)
                    os.environ[key.strip()] = value.strip()

load_env()

API_KEY = os.getenv('ALPHA_VANTAGE_API_KEY', '')
DB_PATH = os.getenv('DB_PATH', 'data/trading_data.db')

def init_database(db_path: str):
    """Initialize database connection - tables are created by C++ server."""
    conn = sqlite3.connect(db_path)
    # Tables are created by the C++ server, so we just return the connection
    return conn

def fetch_stock_data(symbol: str, api_key: str) -> Optional[Dict]:
    """Fetch real-time stock quote from Alpha Vantage."""
    url = f"https://www.alphavantage.co/query"
    params = {
        'function': 'GLOBAL_QUOTE',
        'symbol': symbol,
        'apikey': api_key
    }
    
    try:
        response = requests.get(url, params=params, timeout=10)
        response.raise_for_status()
        data = response.json()
        
        if 'Global Quote' in data:
            quote = data['Global Quote']
            return {
                'symbol': symbol,
                'price': float(quote.get('05. price', 0)),
                'change_percent': float(quote.get('10. change percent', '0%').rstrip('%')),
                'volume': int(quote.get('06. volume', 0)),
                'timestamp': datetime.now().isoformat()
            }
        else:
            print(f"⚠️  No data returned for {symbol}")
            return None
    except Exception as e:
        print(f"❌ Error fetching data for {symbol}: {e}")
        return None

def fetch_time_series(symbol: str, api_key: str, days: int = 30) -> List[Dict]:
    """Fetch historical time series data."""
    url = f"https://www.alphavantage.co/query"
    params = {
        'function': 'TIME_SERIES_DAILY',
        'symbol': symbol,
        'apikey': api_key,
        'outputsize': 'compact' if days <= 100 else 'full'
    }
    
    try:
        response = requests.get(url, params=params, timeout=10)
        response.raise_for_status()
        data = response.json()
        
        if 'Time Series (Daily)' in data:
            time_series = data['Time Series (Daily)']
            results = []
            for date, values in list(time_series.items())[:days]:
                results.append({
                    'symbol': symbol,
                    'date': date,
                    'open': float(values['1. open']),
                    'high': float(values['2. high']),
                    'low': float(values['3. low']),
                    'close': float(values['4. close']),
                    'volume': int(values['5. volume'])
                })
            return results
        else:
            print(f"⚠️  No time series data for {symbol}")
            return []
    except Exception as e:
        print(f"❌ Error fetching time series for {symbol}: {e}")
        return []

def store_stock_data(conn: sqlite3.Connection, data: Dict):
    """Store stock quote data in database using ohlcv_data table."""
    cursor = conn.cursor()
    try:
        # Store as OHLCV data with current price as close
        cursor.execute('''
            INSERT OR REPLACE INTO ohlcv_data (symbol, timestamp, open, high, low, close, volume)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        ''', (data['symbol'], data['timestamp'], data['price'], data['price'], 
              data['price'], data['price'], data['volume']))
        conn.commit()
        print(f"✓ Stored quote data for {data['symbol']}")
    except Exception as e:
        print(f"❌ Error storing data for {data['symbol']}: {e}")

def store_time_series(conn: sqlite3.Connection, data: List[Dict]):
    """Store time series data in database using ohlcv_data table."""
    cursor = conn.cursor()
    try:
        for row in data:
            cursor.execute('''
                INSERT OR REPLACE INTO ohlcv_data (symbol, timestamp, open, high, low, close, volume)
                VALUES (?, ?, ?, ?, ?, ?, ?)
            ''', (row['symbol'], row['date'], row['open'], row['high'], 
                  row['low'], row['close'], row['volume']))
        conn.commit()
        print(f"✓ Stored {len(data)} time series records")
    except Exception as e:
        print(f"❌ Error storing time series: {e}")

def fetch_news(symbol: str, api_key: str, max_articles: int = 10) -> List[Dict]:
    """Fetch news articles for a symbol."""
    url = f"https://www.alphavantage.co/query"
    params = {
        'function': 'NEWS_SENTIMENT',
        'tickers': symbol,
        'apikey': api_key,
        'limit': max_articles
    }
    
    try:
        response = requests.get(url, params=params, timeout=10)
        response.raise_for_status()
        data = response.json()
        
        if 'feed' in data:
            articles = []
            for item in data['feed'][:max_articles]:
                articles.append({
                    'symbol': symbol,
                    'title': item.get('title', ''),
                    'url': item.get('url', ''),
                    'summary': item.get('summary', ''),
                    'source': item.get('source', ''),
                    'published_at': item.get('time_published', ''),
                    'sentiment_score': item.get('overall_sentiment_score', 0.0)
                })
            return articles
        else:
            print(f"⚠️  No news data for {symbol}")
            return []
    except Exception as e:
        print(f"❌ Error fetching news for {symbol}: {e}")
        return []

def store_news(conn: sqlite3.Connection, articles: List[Dict]):
    """Store news articles in database using news_articles table."""
    cursor = conn.cursor()
    try:
        for article in articles:
            # Generate a unique article_id from URL or title
            article_id = article['url'] if article['url'] else article['title'][:100]
            cursor.execute('''
                INSERT OR REPLACE INTO news_articles (article_id, title, content, source, published_time, symbol)
                VALUES (?, ?, ?, ?, ?, ?)
            ''', (article_id, article['title'], article['summary'],
                  article['source'], article['published_at'], article['symbol']))
        conn.commit()
        print(f"✓ Stored {len(articles)} news articles")
    except Exception as e:
        print(f"❌ Error storing news: {e}")

def main():
    parser = argparse.ArgumentParser(description='Ingest market data from Alpha Vantage')
    parser.add_argument('--symbols', nargs='+', required=True, help='Stock symbols to ingest')
    parser.add_argument('--days', type=int, default=30, help='Number of days of historical data')
    parser.add_argument('--news', action='store_true', help='Also ingest news articles')
    parser.add_argument('--max-articles', type=int, default=10, help='Maximum articles per symbol')
    
    args = parser.parse_args()
    
    if not API_KEY:
        print("❌ ERROR: ALPHA_VANTAGE_API_KEY not set in .env file")
        return 1
    
    print(f"📊 Ingesting data for symbols: {', '.join(args.symbols)}")
    print(f"📅 Period: {args.days} days")
    if args.news:
        print(f"📰 News: Up to {args.max_articles} articles per symbol")
    print()
    
    # Initialize database
    conn = init_database(DB_PATH)
    
    # Ingest data for each symbol
    for symbol in args.symbols:
        print(f"\n🔄 Processing {symbol}...")
        
        # Fetch and store real-time quote
        print(f"  Fetching real-time quote...")
        quote_data = fetch_stock_data(symbol, API_KEY)
        if quote_data:
            store_stock_data(conn, quote_data)
        
        # Fetch and store historical data
        if args.days > 0:
            print(f"  Fetching historical data ({args.days} days)...")
            time_series = fetch_time_series(symbol, API_KEY, args.days)
            if time_series:
                store_time_series(conn, time_series)
        
        # Fetch and store news
        if args.news:
            print(f"  Fetching news articles...")
            articles = fetch_news(symbol, API_KEY, args.max_articles)
            if articles:
                store_news(conn, articles)
    
    conn.close()
    print(f"\n✅ Data ingestion complete!")
    return 0

if __name__ == '__main__':
    sys.exit(main())
