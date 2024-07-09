import { OpenAI } from "llamaindex/llm/openai";

const llm = new OpenAI({ model: "gpt-4o" });

const response = await llm.chat({
  messages: [{ content: "What is the C++ FQA?", role: "user" }],
});

console.log(response.message.content);
