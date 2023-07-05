import os
from twitchio.ext import commands

class Bot(commands.Bot):
  def __init__(self):
    super().__init__(
        token=os.environ['ACCESS_TOKEN'],
        client_id=os.environ['CLIENT_ID'],
        nick=os.environ['BOT_NICK'],
        prefix=os.environ['BOT_PREFIX'],
        initial_channels=[os.environ['CHANNEL']],
    )

  async def event_ready(self):
      print(f'Ready | {self.nick}')

  async def event_message(self, message):
    if message.author:
      print(f'{message.timestamp} - {message.author.name} -  {message.content}')
      if "xdd" == message.content and message.author.name == "grencez":
        await message.channel.send("xdd")
    else:
      print(f'{message.timestamp} - [Unknown Author] -  {message.content}')

# bot.py
if __name__ == "__main__":
  print(os.environ['BOT_NICK'])
  bot = Bot()
  bot.run()

