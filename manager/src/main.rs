use std::io::Write;
use zmq;
use rmp_serde::{Deserializer, Serializer};
use serde::{Serialize, Deserialize};

// Struct representing a Job (to be sent to the Worker)
#[derive(Serialize, Deserialize, Debug)]
struct Job {
    prompt: String,
    steps: i32,
    scheduler: String,
}

// Struct representing an Image (received from the Worker)
#[derive(Serialize, Deserialize, Debug)]
struct Image {
    image_data: Vec<u8>,  // Binary data for the image
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Create a ZeroMQ context and socket
    let context = zmq::Context::new();
    let socket = context.socket(zmq::REQ)?;  // REQ for sending requests
    socket.connect("tcp://localhost:5555")?;  // Connect to the worker server

    println!("Manager is ready, sending job to worker...");

    // Create a Job to send to the Worker
    let job = Job {
        prompt: String::from("A beautiful sunset over the ocean"),
        steps: 50,
        scheduler: String::from("euler_a"),
    };

    // Serialize the Job into MessagePack format
    let mut buf = Vec::new();
    job.serialize(&mut Serializer::new(&mut buf))?;

    // Send the job to the worker
    socket.send(&buf, 0)?;
    println!("Job sent. Waiting for image response...");

    // Receive the response (image) from the Worker
    let reply = socket.recv_msg(0)?;

    // Deserialize the image response using MessagePack
    let mut de = Deserializer::new(&reply[..]);
    let image: Image = Deserialize::deserialize(&mut de)?;

    println!("Received image data. Image size: {} bytes", image.image_data.len());

    // Here you would save the image or perform further processing.
    // For example, we save the image to a file:
    let mut outfile = std::fs::File::create("generated_image.bin")?;
    outfile.write_all(&image.image_data)?;

    println!("Image saved to 'generated_image.bin'");

    Ok(())
}
