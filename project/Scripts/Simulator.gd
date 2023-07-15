extends Node2D

# Get audio player
onready var player: AudioStreamPlayer = $Audio

# Get generator stream
onready var stream: AudioStreamGenerator = player.stream

# Get generator playback
onready var playback: AudioStreamGeneratorPlayback = player.get_stream_playback()

# Simulator reference
var sim: EngineSimulator

# Audio generator
var gen: EngineAudioGenerator

# Starter
var starter: float = 0.0

# Audio buffer size
export var buffer_size: float = 0.5

# Called on ready
func _ready() -> void:
	sim = EngineSimulator.new()
	gen = EngineAudioGenerator.new()

	gen.simulator = sim
	gen.stream = stream
	gen.playback = playback

	# Play the audio
	player.play()

	# Simulate 0.1 seconds
	simulate(0.1)

# Called every frame
func _process(delta: float) -> void:
	starter = Input.get_action_strength("ui_accept")
	update()

# Simulate the engine
func simulate(delta: float) -> void:
	# Get simulation rate
	var rate: float = stream.mix_rate

	# Get number of frames
	var frames: int = floor(rate * delta)

	# Get available frames
	var available: int = playback.get_frames_available()

	# Get min frames
	frames = min(frames, available)

	# Get delta time
	var dt: float = 1.0 / rate
	# dt *= 0.1

	# Resize the audio buffer size
	sim.audio_buffer_size = floor(buffer_size * rate)
	sim.audio_sample_rate = stream.mix_rate

	# Set the starter factor
	sim.starter_factor = starter

	# Process the engine
	sim.simulate(dt, frames)

	# Push frames
	gen.push_frames(frames)

# Called every physics frame
func _physics_process(delta: float) -> void:
	# Simulate engine and process audio
	simulate(delta)

# Draw the simulation
func _draw() -> void:
	var rid: RID = get_canvas_item()
	sim.draw_simulation(rid)
