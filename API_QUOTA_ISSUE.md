# OpenAI API Quota Issue

## Problem

The tests are failing because the OpenAI API key has exceeded its quota:

```
[ERROR] OpenAI API error: You exceeded your current quota, please check your plan and billing details.
```

## Root Cause

The OpenAI API key in your `.env` file has reached its usage limit. This affects:
1. **Embedding generation** - Used for vector search
2. **LLM responses** - Used for generating answers to queries

## Solutions

### Option 1: Add Credits to OpenAI Account (Recommended)

1. Go to [OpenAI Platform](https://platform.openai.com/)
2. Navigate to **Billing** → **Add Payment Method**
3. Add credits to your account
4. The API key will work again immediately

### Option 2: Use a Different API Key

1. Generate a new API key from [OpenAI API Keys](https://platform.openai.com/api-keys)
2. Update your `.env` file:
   ```bash
   OPENAI_API_KEY=your_new_api_key_here
   ```
3. Restart the server

### Option 3: Use a Different Model (Temporary Workaround)

If you want to test with a cheaper model while fixing the quota issue, you can temporarily change the model in the code:

**For LLM responses** (`src/rag/rag_agent.cpp`):
```cpp
request_json["model"] = "gpt-3.5-turbo";  // Instead of "gpt-4"
```

**For embeddings** (`src/vectorization/embedding_service.cpp`):
```cpp
request_json["model"] = "text-embedding-ada-002";  // Cheaper model
```

### Option 4: Mock Responses for Testing (Development Only)

For development/testing without API calls, you could add mock responses. However, this defeats the purpose of testing the RAG system.

## Current Status

After the latest fixes:
- ✅ Error handling improved - API errors are now properly logged
- ✅ System continues without embeddings (graceful degradation)
- ❌ LLM responses still fail when quota is exceeded (expected behavior)

## Verification

Once you've resolved the quota issue:

1. **Check API key status:**
   ```bash
   curl https://api.openai.com/v1/models \
     -H "Authorization: Bearer $OPENAI_API_KEY"
   ```

2. **Restart the server:**
   ```bash
   ./build/rag_agent_server
   ```

3. **Test again:**
   ```bash
   python3 scripts/test_client.py
   ```

## Expected Behavior After Fix

Once the quota issue is resolved, you should see:
- ✅ Embedding generation succeeds (or continues without it)
- ✅ LLM responses generated successfully
- ✅ All test cases pass

## Cost Considerations

- **GPT-4**: More expensive but better quality
- **GPT-3.5-turbo**: Cheaper alternative, good for testing
- **Embeddings**: Relatively inexpensive
- **Rate limits**: Free tier has strict limits

## Next Steps

1. **Immediate**: Add credits to OpenAI account or use a new API key
2. **Short-term**: Consider using GPT-3.5-turbo for testing to reduce costs
3. **Long-term**: Implement caching to reduce API calls
4. **Alternative**: Consider using other LLM providers (Anthropic, Cohere, etc.)

