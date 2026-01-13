const messageForm = document.getElementById('message-form');
const messageInput = document.getElementById('message-input');
const chatContainer = document.getElementById('chat-container');

function appendMessage(text, isUser) {
  const messageDiv = document.createElement('div');
  messageDiv.className = isUser ? 'message user-message' : 'message assistant-message';
  messageDiv.innerText = text;
  chatContainer.appendChild(messageDiv);
  chatContainer.scrollTop = chatContainer.scrollHeight;
}

messageForm.addEventListener('submit', async event => {
  event.preventDefault();

  const message = messageInput.value;
  if (!message) return;

  messageInput.value = '';
  appendMessage(message, true);

  try {
    const response = await fetch('/chat', {
      method: 'POST',
      body: JSON.stringify({ message }),
      headers: {
        'Content-Type': 'application/json'
      }
    });

    if (response.ok) {
      const data = await response.json();
      if (data.reply) {
        appendMessage(data.reply, false);
      }
    } else {
      console.error('Error fetching chat response');
    }
  } catch (error) {
    console.error('Network error:', error);
  }
});
