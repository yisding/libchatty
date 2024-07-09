from langchain_openai import ChatOpenAI

model = ChatOpenAI(model="gpt-4o")

from langchain_core.messages import HumanMessage, SystemMessage

messages = [
    HumanMessage(content="What is the C++ FQA?"),
]

result = model.invoke(messages)

from langchain_core.output_parsers import StrOutputParser

parser = StrOutputParser()

print(parser.invoke(result))