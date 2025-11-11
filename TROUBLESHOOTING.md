# Troubleshooting Guide

## Test Failures - INTERNAL Errors

If you're seeing test failures with `StatusCode.INTERNAL`, here are the common causes and fixes:

### Issue 1: Embedding Generation Failing

**Symptoms:**
- `[ERROR] Failed to generate query embedding`
- All RAG queries failing

**Causes:**
1. Invalid OpenAI API key
2. API rate limiting
3. Network connectivity issues
4. API response format changes

**Fixes:**
1. Verify API key in `.env` file:
   ```bash
   cat .env | grep OPENAI_API_KEY
   ```

2. Check API key is valid:
   ```bash
   curl https://api.openai.com/v1/models \
     -H "Authorization: Bearer $OPENAI_API_KEY"
   ```

3. Check server logs for detailed error messages:
   ```bash
   tail -f logs/rag_agent.log
   ```

4. The system now continues without embeddings (works but without vector search context)

### Issue 2: Stock Data Fetching Failing

**Symptoms:**
- `[ERROR] Failed to fetch stock quote for AAPL`
- `[WARNING] Alpha Vantage API rate limit`

**Causes:**
1. Alpha Vantage API rate limiting (5 calls/min on free tier)
2. Invalid API key
3. Network issues

**Fixes:**
1. Wait 1 minute between API calls
2. Use a premium Alpha Vantage API key
3. The system now continues without price data (generates summary anyway)

### Issue 3: Volatility Calculation Failing

**Symptoms:**
- `[ERROR] Failed to fetch volatility for AAPL`
- `Fetched 0 data points for AAPL`

**Causes:**
1. No historical data in database
2. API rate limiting
3. Invalid date format

**Fixes:**
1. Ingest more historical data:
   ```bash
   python3 scripts/ingest_data.py --symbols AAPL --days 30
   ```

2. Use a valid date format (YYYY-MM-DD)

3. The system now continues without volatility data

## Current Status

After the latest fixes, the system is more resilient:

✅ **Works without embeddings** - RAG queries work even if embedding generation fails
✅ **Works without price data** - Can generate summaries without real-time quotes
✅ **Works without volatility data** - Can explain volatility without historical data
✅ **Better error logging** - More detailed error messages in logs

## Testing After Fixes

1. **Restart the server:**
   ```bash
   # Stop current server (Ctrl+C)
   ./build/rag_agent_server
   ```

2. **Run tests:**
   ```bash
   python3 scripts/test_client.py
   ```

3. **Check logs:**
   ```bash
   tail -f logs/rag_agent.log
   ```

## Expected Behavior

After the fixes, you should see:

- ✅ Server starts successfully
- ✅ Some warnings about missing data (expected)
- ✅ RAG queries work even with warnings
- ✅ LLM responses generated successfully

The system will work with degraded functionality:
- Without embeddings: No vector search context, but LLM still responds
- Without price data: Generic summary without current price
- Without volatility: General explanation without specific volatility value

## Next Steps

1. **Verify API keys are working:**
   - Test OpenAI API key
   - Test Alpha Vantage API key

2. **Ingest more data:**
   ```bash
   python3 scripts/ingest_data.py --symbols AAPL MSFT GOOGL --days 30 --news
   ```

3. **Install FAISS** (optional but recommended):
   - Enables full vector search capabilities
   - Better context retrieval

4. **Monitor logs:**
   - Check for rate limiting warnings
   - Verify API responses

