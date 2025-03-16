use std::collections::{HashMap, VecDeque};
use std::io::Write;
use std::time::{Duration, Instant};
use zmq;
use rmp_serde::{Deserializer, Serializer};
use serde::{Serialize, Deserialize};

// --- Message Types ---
#[derive(Serialize, Deserialize, Debug)]
enum Message {
    RegisterWorker,          // Worker registers with the manager
    JobAssign(Job),          // Manager assigns a job to a worker
    JobResult(Image),        // Worker sends back generated image
    Heartbeat,               // Worker sends a heartbeat
}

// --- Job Struct ---
#[derive(Serialize, Deserialize, Debug)]
struct Job {
	id: i32,
    prompt: String,
    steps: i32,
    scheduler: String,
}

// --- Image Struct ---
#[derive(Serialize, Deserialize, Debug)]
struct Image {
    image_data: Vec<u8>,  // Binary image data
}

// --- Worker Tracking ---
struct Worker {
    id: String,               // Worker ID
    socket: zmq::Socket,      // ZMQ Socket for communication
    last_seen: Instant,       // Last heartbeat timestamp
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let context = zmq::Context::new();
    let manager_socket = context.socket(zmq::ROUTER)?;  // ROUTER to handle multiple workers
    manager_socket.bind("tcp://*:5555")?;

    let mut workers: HashMap<String, Worker> = HashMap::new();  // Active workers
    let mut job_queue: VecDeque<Job> = VecDeque::new();  // Job queue

    // Add jobs to the queue
    for i in 1..=6 {
        job_queue.push_back(Job {
			id: i,
            prompt: format!("Image {}", i),
            steps: 50,
            scheduler: String::from("euler_a"),
        });
    }

    println!("Manager started. Waiting for workers...");

    loop {
        let mut poll_items = [manager_socket.as_poll_item(zmq::POLLIN)];

        // Wait for worker messages
        zmq::poll(&mut poll_items, 1000)?; // 1 sec timeout

        if poll_items[0].is_readable() {
            let identity = manager_socket.recv_string(0)?;  // Worker identity
            let identity = identity.unwrap();
            let message = manager_socket.recv_msg(0)?;  // Worker message

            let mut de = Deserializer::new(&message[..]);
            let msg: Message = Deserialize::deserialize(&mut de)?;

            match msg {
                Message::RegisterWorker => {
					println!("Worker {} registered!", identity);
					let worker_socket = context.socket(zmq::DEALER)?;
					worker_socket.connect(&format!("tcp://{}", identity))?;
					workers.insert(identity.clone(), Worker {
						id: identity.clone(),
						socket: worker_socket,
						last_seen: Instant::now(),
					});

					// Assign a job if available
					if let Some(job) = job_queue.pop_front() {
						send_message(&manager_socket, &identity, Message::JobAssign(job))?;
					}
				}
                Message::JobResult(image) => {
					println!("Worker {} finished a job!", identity);
					save_image(&format!("image_{}.bin", identity), &image)?;

					// Assign a new job if available
					if let Some(job) = job_queue.pop_front() {
						send_message(&manager_socket, &identity, Message::JobAssign(job))?;
					} else {
						println!("All jobs completed!");
					}
				}
				Message::Heartbeat => {
					if let Some(worker) = workers.get_mut(&identity) {
						worker.last_seen = Instant::now();
					}
				}
				Message::JobAssign(_) => unreachable!(),
            }
        }

        // Remove unresponsive workers
        let now = Instant::now();
        workers.retain(|_, worker| now.duration_since(worker.last_seen) < Duration::from_secs(5));
    }
}

// Send a message to a worker
fn send_message(socket: &zmq::Socket, identity: &str, msg: Message) -> Result<(), Box<dyn std::error::Error>> {
    let mut buf = Vec::new();
    msg.serialize(&mut Serializer::new(&mut buf))?;
    socket.send(identity, zmq::SNDMORE)?;
    socket.send(&buf, 0)?;
    Ok(())
}

// Save image data
fn save_image(filename: &str, image: &Image) -> std::io::Result<()> {
    let mut file = std::fs::File::create(filename)?;
    file.write_all(&image.image_data)?;
    println!("Saved image to {}", filename);
    Ok(())
}
