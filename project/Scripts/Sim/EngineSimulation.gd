extends Node2D

# A single gear
class Gear:
	extends Reference

	# Gear radius
	var radius: float = 0.5

	# Tooth radius
	var tooth_radius: float = 0.1

	# Number of tooths
	var tooth_count: int = 8

	# Tooth size ratio
	var tooth_size_ratio: float = 0.5

	# Current position and rotation
	var position: Vector2 = Vector2.ZERO
	var rotation: float = 0.0

	# Joint position
	var joint_position: Vector2 = Vector2.ZERO

	# Center of mass
	var com: Vector2 = Vector2.ZERO

	# Applied position and rotation
	var applied_position: Vector2 = Vector2.ZERO
	var applied_rotation: float = 0.0

	# Current velocity
	var linear_velocity: Vector2 = Vector2.ZERO
	var angular_velocity: float = 0.0

	# Applied velocity
	var applied_linear_velocity: Vector2 = Vector2.ZERO
	var applied_angular_velocity: float = 0.0

	# Next gear
	var next_gear: Reference = null

	# Create a gear
	func _init(data: Dictionary = {}) -> void:
		self.radius = data.get("radius", self.radius)
		self.tooth_radius = data.get("tooth_radius", self.tooth_radius)
		self.tooth_count = data.get("tooth_count", self.tooth_count)
		self.tooth_size_ratio = data.get("tooth_size_ratio", self.tooth_size_ratio)
		self.position = data.get("position", self.position)
		self.rotation = data.get("rotation", self.rotation)
		self.com = data.get("com", self.com)
		self.linear_velocity = data.get("linear_velocity", self.linear_velocity)
		self.angular_velocity = data.get("angular_velocity", self.angular_velocity)
	
		self.joint_position = self.position
		self.joint_position = data.get("joint_position", self.joint_position)

	# Apply central impulse
	func apply_central_impulse(j: Vector2, positional: bool = false) -> void:
		# j = Vector2.ZERO
		if positional:
			applied_position += j
		else:
			applied_linear_velocity += j 
	
	# Apply torque
	func apply_torque(j: float, positional: bool = false) -> void:
		if positional:
			applied_rotation += j
		else:
			applied_angular_velocity += j 

	# Apply impulse at global position
	func apply_impulse(pos: Vector2, j: Vector2, positional: bool = false) -> void:
		pos -= position
		apply_central_impulse(j, positional)
		apply_torque(pos.cross(j), positional)
	
	# Get velocity at global position
	func get_point_velocity(pos: Vector2) -> Vector2:
		pos -= position
		return Vector2(
			linear_velocity.x - angular_velocity * pos.y,
			linear_velocity.y + angular_velocity * pos.x
		)
	
	# Commit applied forces
	func commit_applied_forces() -> void:
		position += applied_position
		rotation += applied_rotation
		linear_velocity += applied_linear_velocity
		angular_velocity += applied_angular_velocity

		# Reset applied forces
		applied_position = Vector2.ZERO
		applied_rotation = 0.0
		applied_linear_velocity = Vector2.ZERO
		applied_angular_velocity = 0.0

# Gear system builder
class GearSystemBuilder:
	extends Reference

	# First gear
	var first_gear: Gear

	# Current gear
	var current_gear: Gear

	# Next gear
	var next_gear: Gear

	# Initializes
	func _init() -> void:
		first_gear = Gear.new()
		current_gear = first_gear
	
	# Append gear
	func append_gear() -> Reference:
		# Has next gear
		if next_gear:
			current_gear.next_gear = next_gear
		
		# Set current gear and create the next gear
		current_gear = next_gear
		next_gear = Gear.new()

		# Return self for chaining methods
		return self

	# Build the gear
	func build() -> Gear:
		return first_gear

# Starter factor
var starter: float

# Audio buffer to visualize
var buffer: PoolVector2Array

# Audio buffer size
var buffer_size: int

# Buffer position
var buffer_pos: int

# Current time
var time: float

# Previous air velocity
var prev_vel: float

# Current time scale
var time_scale: float = 1.0

# Main gear
var gear: Gear

# Audio player node
onready var audio_player: AudioStreamPlayer = $Audio

# Get generator
onready var generator: AudioStreamGenerator = audio_player.stream

# Get generator playback
onready var generator_playback: AudioStreamGeneratorPlayback = audio_player.get_stream_playback()

# --Physics settings--
# Spring force
export var spring_force: float = 4096.0

# Spring drag
export var spring_drag: float = 8.0

# Friction between gears
export var friction: float = 64.0

# Gear movement volume
export var volume: float = 1.0

# --Debug--
# Buffer length
export var buffer_length: float = 0.5

# Called on ready
func _ready() -> void:
	# Play the audio
	audio_player.play()

	# Get buffer size
	buffer_size = int(buffer_length * generator.mix_rate)

	# Create the buffer
	buffer.resize(buffer_size)

	# Create gears
	var g0: Gear = Gear.new({
		"tooth_count": 32,
		"radius": 0.5,
		"tooth_radius": 0.05,
		"tooth_size_ratio": 0.4,
		"position": Vector2(0.0, 0.0),
		"com": Vector2(0.001, 0.0)
	})
	var g1: Gear = Gear.new({
		"tooth_count": 32,
		"radius": 0.5,
		"tooth_radius": 0.05,
		"tooth_size_ratio": 0.4,
		"position": Vector2(0.5 + 0.5 + 0.05, 0.0).rotated(PI * 0.25),
		"com": Vector2(0.001, -0.001)
	})
	g0.next_gear = g1
	# g0.angular_velocity = 500.0 * TAU / 60.0

	# Set the main gear
	gear = g0

	time_scale = 0.01

	engine_process(0.0)

# Called every frame
func _process(delta: float) -> void:
	starter = Input.get_action_strength("ui_accept")

	update()

# Called every physics frame
func _physics_process(delta: float) -> void:
	engine_process(delta)

# Process the engine
func engine_process(delta: float) -> void:
	if Input.is_action_just_pressed("ui_cancel"):
		if time_scale > 0.5:
			time_scale = 0.01
		else:
			time_scale = 1.0

	# Get number of audio frames
	var frames: int = generator_playback.get_frames_available()

	# Get minimum frames
	frames = min(frames, int(generator.mix_rate * delta))

	# Get hearing distance
	var hear_dist: float = 8.0

	# Get attenuation factor
	var att: float = 1.0 / (1.0 + hear_dist * hear_dist)

	# Has frames to render
	if frames > 0:
		# Get each frame delta
		var dt: float = 1.0 / generator.mix_rate
		# dt = delta
		# frames = 8
		# dt /= frames
		# dt = delta
		# frames = 1
		dt *= time_scale

		# For each frame
		for i in frames:
			# Increase time
			time += dt

			# Current updated gear
			var gear: Gear = self.gear
			
			# Accelerate gear angular velocity to 500 rpm
			var anv: float = 500.0 * TAU / 60.0
			var acc: float = (8000.0 * TAU / 60.0)
			var dec: float = (1000.0 * TAU / 60.0)
			acc = acc * dt * starter
			dec = dec * dt
			# gear.angular_velocity += clamp(-gear.angular_velocity, -dec, dec)
			gear.angular_velocity += clamp(anv - gear.angular_velocity, -acc, acc)

			# Get average system velocity
			var vel: Vector2 = Vector2.ZERO
			var vel_div: float = 0.0

			# While has a gear to update
			while gear:
				# Get next gear
				var next_gear: Gear = gear.next_gear

				# Move gear
				gear.position += gear.linear_velocity * dt
				gear.rotation += gear.angular_velocity * dt
				
				# Apply spring force
				gear.apply_central_impulse((gear.joint_position - gear.position) * clamp(spring_force * dt, 0.0, 1.0))
				gear.apply_central_impulse(-gear.linear_velocity * clamp(spring_drag * dt, 0.0, 1.0))

				# Has next gear
				if next_gear:
					gears_transmission(gear, next_gear, dt)
				
				# Apply torque drag
				if gear != self.gear:
					gear.apply_torque(-gear.angular_velocity * clamp(8.0 * dt, 0.0, 1.0))

				# Commit applied force of this gear
				gear.commit_applied_forces()

				# Add resultant velocity
				if gear == self.gear:
					vel += gear.get_point_velocity(gear.position)
					vel_div += 1.0

				# Set next gear
				gear = next_gear

			# Get average velocity
			# vel /= vel_div
			# vel = self.gear.linear_velocity

			# Get signal
			var signal: float = vel.x

			# Create frame
			var frame: Vector2 = Vector2.ONE * signal * volume * att

			# Push frame to generator
			generator_playback.push_frame(frame)

			# Add frame to buffer
			buffer[buffer_pos] = frame
			buffer_pos = (buffer_pos + 1) % buffer_size

# Simulate transmission between gears
func gears_transmission(g0: Gear, g1: Gear, dt: float) -> void:
	# Get both gears position
	var pos0: Vector2 = g0.position + g0.com.rotated(g0.rotation)
	var pos1: Vector2 = g1.position + g1.com.rotated(g1.rotation)

	# Get offset between gears
	var off: Vector2 = pos1 - pos0

	# Get distance between gears
	var dst: float = off.length_squared()

	# Get minimum dst
	var mdst: float = g0.radius + g1.radius + g0.tooth_radius + g1.tooth_radius

	# Is intersecting
	if dst > 0.0 and dst <= mdst * mdst:
		# Get distance square rooted
		dst = sqrt(dst)

		# Get normal
		var normal: Vector2 = off / dst

		# Get collision position
		var col_pos: Vector2 = pos1 - normal * g1.radius

		# Get both gears tooth pitch
		var ph0: float = TAU / g0.tooth_count
		var ph1: float = TAU / g1.tooth_count

		# Get both gears tooth width
		var w0: float = ph0 * g0.tooth_size_ratio
		var w1: float = ph1 * g1.tooth_size_ratio

		# Get both gears tooth time
		var t0: float = g0.rotation
		var t1: float = g1.rotation

		# Fract both times
		t0 = fposmod(t0 + ph0 * 0.5, ph0) - ph0 * 0.5
		t1 = fposmod(t1 + ph1 * 0.5 + PI, ph1) - ph1 * 0.5
		
		# Get offset between tooths
		var th_off: float = t1 - t0

		# Get collision distance
		var col_dst: float = w0 * 0.5 + w1 * 0.5

		# Are colliding
		if abs(th_off) <= col_dst:
			# Get collision velocity
			var vel: Vector2 = g0.get_point_velocity(col_pos) - g1.get_point_velocity(col_pos)

			# Get friction force
			var fric: Vector2 = vel - normal * normal.dot(vel)
			fric *= clamp(friction * dt, 0.0, 1.0)

			# Apply friction between gears
			g0.apply_impulse(col_pos, -fric * 0.5)
			g1.apply_impulse(col_pos,  fric * 0.5)

# Draw a single gear
func draw_gear(gear: Gear) -> void:
	var c: Color = Color(1.0, 1.0, 1.0, 0.2)

	# Get gear position
	var gear_pos: Vector2 = (gear.position + gear.com.rotated(gear.rotation)) * 100.0

	# Reset the draw transform
	draw_set_transform_matrix(Transform2D.IDENTITY)

	# Draw joint
	draw_circle(gear.joint_position * 100.0, gear.radius * 10.0, c)
	draw_circle(gear.position * 100.0, gear.radius * 10.0, c)

	# Set the draw transform
	draw_set_transform(gear_pos, gear.rotation, Vector2.ONE)

	# Draw the gear radius
	draw_circle(Vector2.ZERO, gear.radius * 100.0, c)

	# Get gear circumference
	var circ: float = TAU * gear.radius

	# Get tooth size
	var th_size: float = (circ / gear.tooth_count) * gear.tooth_size_ratio

	# Get angle spacing
	var a: float = TAU / gear.tooth_count

	# Calculate gear tooth offset
	var th_of: float = -gear.tooth_radius

	# For each tooth
	for i in gear.tooth_count:
		# Transform by tooth rotation
		draw_set_transform(gear_pos, gear.rotation + a * i, Vector2.ONE)

		# Draw tooth
		draw_rect(Rect2(
			gear.radius * 90.0, -th_size * 50.0,
			gear.radius * 10.0 + gear.tooth_radius * 100.0, th_size * 100.0
		), c)
	
	# Has a next gear
	if gear.next_gear:
		# Draw next gear
		draw_gear(gear.next_gear)

# Called to draw
func _draw() -> void:
	draw_gear(self.gear)






