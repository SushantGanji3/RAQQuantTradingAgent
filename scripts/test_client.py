#!/usr/bin/env python3
"""
Test gRPC client for RAG Agent Server.
"""

import grpc
import sys
import os

# Add python_client to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'python_client'))

try:
    import rag_service_pb2
    import rag_service_pb2_grpc
except ImportError as e:
    print(f"❌ Error importing protobuf files: {e}")
    print("Please generate Python protobuf files first:")
    print("  python3 -m grpc_tools.protoc -I./proto --python_out=./python_client --grpc_python_out=./python_client ./proto/rag_service.proto")
    sys.exit(1)

def test_stock_summary(stub, symbol: str = "AAPL", period: str = "1m"):
    """Test GetStockSummary RPC."""
    print(f"\n📊 Testing GetStockSummary for {symbol}...")
    try:
        request = rag_service_pb2.StockSummaryRequest(
            symbol=symbol,
            period=period,
            max_context_docs=5
        )
        response = stub.GetStockSummary(request)
        
        print(f"✅ Response received:")
        print(f"   Symbol: {response.symbol}")
        print(f"   Current Price: ${response.current_price:.2f}")
        print(f"   Change: {response.change_percent:.2f}%")
        print(f"   Volume: {response.volume:,.0f}")
        print(f"   Summary: {response.summary[:200]}..." if len(response.summary) > 200 else f"   Summary: {response.summary}")
        print(f"   Context Docs: {len(response.context_docs)}")
        return True
    except grpc.RpcError as e:
        print(f"❌ gRPC Error: {e.code()} - {e.details()}")
        return False
    except Exception as e:
        print(f"❌ Error: {e}")
        return False

def test_query_rag(stub, query: str = "What is the current market sentiment for AAPL?"):
    """Test QueryRAG RPC."""
    print(f"\n💬 Testing QueryRAG...")
    print(f"   Query: {query}")
    try:
        request = rag_service_pb2.QueryRequest(
            query=query,
            max_context_docs=5,
            symbols=["AAPL"]
        )
        response = stub.QueryRAG(request)
        
        print(f"✅ Response received:")
        print(f"   Answer: {response.answer[:300]}..." if len(response.answer) > 300 else f"   Answer: {response.answer}")
        print(f"   Confidence: {response.confidence:.2f}")
        print(f"   Context Docs: {len(response.context_docs)}")
        return True
    except grpc.RpcError as e:
        print(f"❌ gRPC Error: {e.code()} - {e.details()}")
        return False
    except Exception as e:
        print(f"❌ Error: {e}")
        return False

def test_explain_volatility(stub, symbol: str = "AAPL", date: str = "2024-11-10"):
    """Test ExplainVolatility RPC."""
    print(f"\n📈 Testing ExplainVolatility for {symbol} on {date}...")
    try:
        request = rag_service_pb2.VolatilityRequest(
            symbol=symbol,
            date=date,
            lookback_days=30
        )
        response = stub.ExplainVolatility(request)
        
        print(f"✅ Response received:")
        print(f"   Symbol: {response.symbol}")
        print(f"   Date: {response.date}")
        print(f"   Volatility: {response.volatility_value:.4f}")
        print(f"   Explanation: {response.explanation[:300]}..." if len(response.explanation) > 300 else f"   Explanation: {response.explanation}")
        print(f"   Context Docs: {len(response.context_docs)}")
        return True
    except grpc.RpcError as e:
        print(f"❌ gRPC Error: {e.code()} - {e.details()}")
        return False
    except Exception as e:
        print(f"❌ Error: {e}")
        return False

def main():
    server_address = os.getenv('SERVER_ADDRESS', 'localhost:50051')
    
    print("=" * 60)
    print("RAG Quant Trading Agent - Test Client")
    print("=" * 60)
    print(f"Connecting to server: {server_address}")
    
    try:
        # Create gRPC channel
        channel = grpc.insecure_channel(server_address)
        stub = rag_service_pb2_grpc.RAGAgentServiceStub(channel)
        
        # Test connection
        print("\n🔍 Testing server connection...")
        try:
            grpc.channel_ready_future(channel).result(timeout=5)
            print("✅ Connected to server!")
        except grpc.FutureTimeoutError:
            print("❌ Connection timeout. Is the server running?")
            print("   Start server with: ./build/rag_agent_server")
            return 1
        
        # Run tests
        success_count = 0
        total_tests = 3
        
        # Test 1: Stock Summary
        if test_stock_summary(stub, "AAPL", "1m"):
            success_count += 1
        
        # Test 2: Query RAG
        if test_query_rag(stub, "What is Apple's current stock performance?"):
            success_count += 1
        
        # Test 3: Explain Volatility
        if test_explain_volatility(stub, "AAPL", "2024-11-10"):
            success_count += 1
        
        # Summary
        print("\n" + "=" * 60)
        print(f"Test Results: {success_count}/{total_tests} tests passed")
        print("=" * 60)
        
        channel.close()
        return 0 if success_count == total_tests else 1
        
    except Exception as e:
        print(f"❌ Fatal error: {e}")
        import traceback
        traceback.print_exc()
        return 1

if __name__ == '__main__':
    sys.exit(main())

