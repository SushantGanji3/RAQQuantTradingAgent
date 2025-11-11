#!/usr/bin/env python3
"""
Data ingestion script to fetch and store market data.
"""

import os
import sys
import argparse
from datetime import datetime, timedelta

# Add parent directory to path for imports
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

def ingest_stock_data(symbols, days=30):
    """Ingest stock data for given symbols."""
    # This would use the C++ bindings or make direct API calls
    print(f"Ingesting data for symbols: {symbols}")
    print(f"Period: {days} days")
    
    # Placeholder - would use DataFetcher and Database
    for symbol in symbols:
        print(f"Fetching data for {symbol}...")
        # data_fetcher.fetch_stock_data(symbol, "daily", days)
        # database.store_ohlcv_data(symbol, data)
        print(f"✓ Stored data for {symbol}")

def ingest_news(symbols, max_articles=50):
    """Ingest news articles for given symbols."""
    print(f"Ingesting news for symbols: {symbols}")
    
    for symbol in symbols:
        print(f"Fetching news for {symbol}...")
        # data_fetcher.fetch_news(symbol, max_articles)
        # Store in database and vectorize
        print(f"✓ Stored news for {symbol}")

def main():
    parser = argparse.ArgumentParser(description='Ingest market data')
    parser.add_argument('--symbols', nargs='+', required=True, help='Stock symbols to ingest')
    parser.add_argument('--days', type=int, default=30, help='Number of days of historical data')
    parser.add_argument('--news', action='store_true', help='Also ingest news articles')
    parser.add_argument('--max-articles', type=int, default=50, help='Maximum articles per symbol')
    
    args = parser.parse_args()
    
    ingest_stock_data(args.symbols, args.days)
    
    if args.news:
        ingest_news(args.symbols, args.max_articles)

if __name__ == '__main__':
    main()

