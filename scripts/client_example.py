#!/usr/bin/env python3
"""
Example gRPC client for RAG Agent.
"""

import grpc
import sys
import os

# Add proto directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'python_client'))

# These would be generated from proto files
# from proto import rag_service_pb2
# from proto import rag_service_pb2_grpc

def query_stock_summary(stub, symbol, period="1m"):
    """Query stock summary."""
    # request = rag_service_pb2.StockSummaryRequest(symbol=symbol, period=period)
    # response = stub.GetStockSummary(request)
    # print(f"Summary for {symbol}:")
    # print(response.summary)
    print(f"Querying stock summary for {symbol}...")
    print("(This requires compiled proto files)")

def query_volatility(stub, symbol, date):
    """Query volatility explanation."""
    # request = rag_service_pb2.VolatilityRequest(symbol=symbol, date=date)
    # response = stub.ExplainVolatility(request)
    # print(f"Volatility explanation for {symbol} on {date}:")
    # print(response.explanation)
    print(f"Querying volatility for {symbol} on {date}...")

def main():
    # Connect to server
    channel = grpc.insecure_channel('localhost:50051')
    # stub = rag_service_pb2_grpc.RAGAgentServiceStub(channel)
    
    print("RAG Agent Client Example")
    print("=" * 50)
    
    # Example queries
    # query_stock_summary(stub, "AAPL", "1m")
    # query_volatility(stub, "AAPL", "2024-01-15")
    
    print("\nNote: This requires:")
    print("1. Compiling proto files to Python")
    print("2. Running the gRPC server")
    print("3. Installing grpcio and grpcio-tools")

if __name__ == '__main__':
    main()

