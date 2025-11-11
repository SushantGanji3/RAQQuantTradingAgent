#pragma once

#include <string>
#include <memory>
#include "rag/rag_agent.h"

void RunServer(const std::string& server_address, std::shared_ptr<rag::agent::RAGAgent> rag_agent);

