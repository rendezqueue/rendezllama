const messageForm = document.getElementById('message-form');
const messageInput = document.getElementById('message-input');
const chatContainer = document.getElementById('chat-container');

messageForm.addEventListener('submit', event => {
	event.preventDefault();

	const message = messageInput.value;
	messageInput.value = '';

	fetch('/chat', {
		method: 'POST',
		body: JSON.stringify({ message }),
		headers: {
			'Content-Type': 'application/json'
		}
	});
});


const pollChat = () => {
	fetch('/chat')
		.then(response => response.json())
		.then(({ reply }) => {
			if (reply) {
				const chatMessage = document.createElement('div');
				chatMessage.innerText = reply;
				chatContainer.appendChild(chatMessage);
			}
		})
		.finally(() => setTimeout(pollChat, 3000));
};

pollChat();
