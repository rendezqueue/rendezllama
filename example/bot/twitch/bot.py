import os
import subprocess
from twitchio.ext import commands

chat_process = subprocess.Popen(
    args=[
      '../../../bld/src/chat/chat',
      '--x_setting',
      'setting.sxproto',
      '--model',
      os.environ['BOT_MODEL'],
    ],
    universal_newlines=True,
    bufsize=1,
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
)

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
    if not message.author:
      print(f'{message.timestamp} - [Unknown Author] -  {message.content}')
      return

    print(f'{message.timestamp} - {message.author.name} -  {message.content}')
    if not message.content.startswith('!gen '):
      print('ignored')
      return
    content = message.content[5:]

    if message.author.name != self.nick:
      chat_process.stdin.write(f'/puts USER:\n')
      chat_process.stdin.write(f'/puts {content}\n')
      chat_process.stdin.write(f'/puts\n')
      chat_process.stdin.write(f'/puts ASSISTANT:\n')
      chat_process.stdin.write(f'/gets 500\n')
      chat_process.stdin.flush()
      s = chat_process.stdout.readline()
      # chat_process.stdin.write(f'/d\n/d\n/d\n/d\n/d\n')
      chat_process.stdin.write(f'/rollforget 5\n')
      chat_process.stdin.flush()
      if len(s) > 500:
        s = s[:500]
      await message.channel.send(s)

# bot.py
if __name__ == '__main__':
  print(os.environ['BOT_NICK'])
  bot = Bot()
  bot.run()

