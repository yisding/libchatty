from langchain_openai import ChatOpenAI
from langchain_core.messages import HumanMessage
from langchain_core.output_parsers import StrOutputParser

model = ChatOpenAI(model="gpt-4o")
messages = [HumanMessage(content="What is the C++ FQA?")]
result = model.invoke(messages)

print(StrOutputParser().invoke(result))
