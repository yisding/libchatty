from llama_index.llms.openai import OpenAI
from llama_index.core.base.llms.types import ChatMessage

llm = OpenAI(model="gpt-4o")
print(llm.chat([ChatMessage.from_str("What is the C++ FQA?", role="user")]).message.content)
