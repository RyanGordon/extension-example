<?hh

<<__Native>>
function shfifo_init(string $name): bool;

<<__Native>>
function shfifo_push(string $queue_name, string $value): bool;

<<__Native>>
function shfifo_pop(string $queue_name): ?string;

class SharedFifo {

	// This will create a shared fifo if it does not already exist. If it does exist then it won't do anything.
	// This is a fast operation
	public function __construct(private string $name) {
		shfifo_init($name);
	}

	// This will push a value onto the shared fifo
	public function push(string $value): bool {
		return shfifo_push($this->name, $value);
	}

	// This will return a string value from the fifo or null if the fifo is empty
	public function pop(): ?string {
		return shfifo_pop($this->name);
	}
}