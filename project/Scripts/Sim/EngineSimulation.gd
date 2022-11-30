extends Node2D

# Single cylinder class
class Cylinder:
	extends Reference

	# Cylinder width
	var width: float = 0.5

	# Cylinder rotation
	var rotation: float = 0.0

	# Cylinder offset
	var offset: float = 0.0

	# Piston width
	var piston_width: float = 0.1

	# Piston top margin
	var piston_margin: float = 0.05

	# Piston crank offset
	var crank_offset: float = 0.0

	# Cylinder length
	var length: float = 1.0

	# Connection rod length
	var rod_length: float = 0.0

	# Current piston position
	var piston_position: float = 0.0

	# Previous piston position
	var piston_prev_position: float = 0.0

	# Current piston velocity
	var piston_velocity: float = 0.0

	# Create a new piston
	func _init(data: Dictionary) -> void:
		self.width = data.get("width", self.width)
		self.rotation = data.get("rotation", self.rotation)
		self.offset = data.get("offset", self.offset)
		self.crank_offset = data.get("crank_offset", self.crank_offset)
		self.piston_width = data.get("piston_width", self.piston_width)
		self.piston_margin = data.get("piston_margin", self.piston_margin)

# Gas factor
var gas: float

# Starter factor
var starter: float

# Current crankshaft rotation
var crank_angle: float

# Current crankshaft angular velocity
var crank_ang_vel: float

# Applied crankshaft angular velocity
var crank_applied_ang_vel: float

# Engine position
var engine_pos: Vector2

# Engine rotation
var engine_rot: float

# Engine velocity
var engine_vel: Vector2

# Engine angular velocity
var engine_ang_vel: float

# Applied engine applied velocity
var engine_applied_vel: Vector2

# Applied engine applied angular velocity
var engine_applied_ang_vel: float

# Cylinders array
var cylinders: Array

# Audio buffer to visualize
var buffer: PoolVector2Array

# Audio buffer size
var buffer_size: int

# Buffer position
var buffer_pos: int

# Is revving
var revving: bool

# Current time
var time: float

# Max piston velocity
var max_piston_vel: float = 0.0

# Audio player node
onready var audio_player: AudioStreamPlayer = $Audio

# Get generator
onready var generator: AudioStreamGenerator = audio_player.stream

# Get generator playback
onready var generator_playback: AudioStreamGeneratorPlayback = audio_player.get_stream_playback()

# Crankshaft radius
export var crankshaft_radius: float = 0.5

# Crankshaft connection radius
export var crankshaft_connection_radius: float = 0.25

# Starter acceleration
export var starter_acceleration: float = 16.0

# Piston drag
export var piston_drag: float = 8.0

# Piston acceleration
export var piston_acceleration: float = 100.0

# Engine spring force
export var engine_spring_force: float = 8.0

# Engine spring dampening
export var engine_spring_damp: float = 0.0

# Max rpm
export var engine_max_rpm: float = 8000.0

# Margin rpm
export var engine_margin_rpm: float = 100.0

# Engine noise
export var engine_noise: OpenSimplexNoise

# Engine noise frequency
export var engine_noise_freq: float = 1024.0

# Engine noise fluctuation
export var engine_noise_fluctuation: float = 0.1

# --Mixing--
# Vibrations volume
export var engine_vibration_volume: float = 0.1

# --Debug--
# Buffer length
export var buffer_length: float = 0.5

# Crankshaft color
export var crankshaft_color: Color = Color(0.5, 0.5, 0.5)

# Crankshaft rotation sticker color
export var crankshaft_sticker_color: Color = Color(1.0, 1.0, 1.0)

# Cylinder walls color
export var cylinder_walls_color: Color = Color(1.0, 0.25, 0.25)

# Piston color
export var piston_color: Color = Color(0.9, 0.9, 0.9)

# Piston joint color
export var piston_joint_color: Color = Color(0.6, 0.6, 0.6)

# Connection rod color
export var connection_rod_color: Color = Color(0.7, 0.7, 0.7)

# Called on ready
func _ready() -> void:
	crank_ang_vel = TAU
	crank_ang_vel = 1.0 * TAU
	# crank_ang_vel = 0.0

	# Play the audio
	audio_player.play()

	# Get buffer size
	buffer_size = int(buffer_length * generator.mix_rate)

	# Create the buffer
	buffer.resize(buffer_size)

	# Initialize the cylinders
	cylinders = []

	# Create the cylinders
	for i in 1:
		var t: float = i / 1.0

		var rot: float = 0.0
		if i % 2 == 0:
			rot = -PI * 0.4
		else:
			rot = PI * 0.4
		# rot = TAU * t
		rot += -PI * 0.5

		var off: float = t * PI
		# off = -rot
		# off = 0.0
		# if i % 2 == 0:
		# 	off = PI * 0.25
		# else:
		# 	off = -PI * 0.25

		cylinders.append(Cylinder.new({
			"width": 0.5,
			"rotation": rot,
			"offset": 0.0,
			# "crank_offset": -t * TAU * 1.0,
			"crank_offset": off,
			"piston_width": 0.2,
			"piston_margin": 0.1
		}))

	engine_process(0.0)

# Called every frame
func _process(delta: float) -> void:
	starter = Input.get_action_strength("ui_accept")

	# # Reset audio
	# if Input.is_action_just_pressed("ui_cancel"):
	# 	audio_player.stop()
	# 	audio_player.play()
	update()

# Called every physics frame
func _physics_process(delta: float) -> void:
	engine_process(delta)

# Process the engine
func engine_process(delta: float) -> void:
	# Get number of cylinders
	var cyl_count: int = cylinders.size()

	# Get audio factor for each cylinder
	var cylf: float = 1.0 / cyl_count

	# For each cylinder
	for i in cyl_count:
		# Get cylinder
		var cyl: Cylinder = cylinders[i]

		# Calculate connection rod length
		cyl.rod_length = crankshaft_connection_radius * 2.0 + cyl.offset

		# Calculate cylinder length
		cyl.length = crankshaft_connection_radius * 2.0 + cyl.piston_width + cyl.piston_margin

	# Get frames
	var frames: int = generator_playback.get_frames_available()

	# Get minimum frames
	frames = min(frames, int(generator.mix_rate * delta))

	# Get hearing distance
	var hear_dist: float = 8.0

	# Get attenuation factor
	var att: float = 1.0 / (1.0 + hear_dist * hear_dist)

	# Get max crankshaft velocity
	var max_crank_vel: float = engine_max_rpm * TAU / 60.0

	# Get margin crankshaft velocity
	var margin_crank_vel: float = engine_margin_rpm * TAU / 60.0

	# Has frames to render
	if frames > 0:
		# Get each frame delta
		var dt: float = 1.0 / generator.mix_rate
		# dt *= 0.1

		# For each frame
		for i in frames:
			# Increase time
			time += dt

			# Reset applied forces
			crank_applied_ang_vel = 0.0
			engine_applied_vel = Vector2.ZERO
			engine_applied_ang_vel = 0.0

			# Increase position
			crank_angle += crank_ang_vel * dt
			crank_angle = fposmod(crank_angle, TAU * 2.0)

			# Transform the engine
			engine_pos += engine_vel * dt
			engine_rot += engine_ang_vel * dt

			# Get noise
			var n: float = engine_noise.get_noise_1d(time * engine_noise_freq) * 0.5 + 0.5
			n = 1.0 - n * engine_noise_fluctuation

			# Apply engine spring
			engine_applied_vel += -engine_pos * engine_spring_force * 1000.0 * dt
			engine_applied_ang_vel += -engine_rot * engine_spring_force * 1000.0 * dt
			engine_applied_vel += -engine_vel * n * clamp(engine_spring_damp * 1000.0 * dt, 0.0, 1.0)
			engine_applied_ang_vel += -engine_ang_vel * n * clamp(engine_spring_damp * 1000.0 * dt, 0.0, 1.0)

			# Apply starter
			crank_applied_ang_vel += starter_acceleration * starter * dt

			# The overall engine velocity
			var velocity: Vector2 = Vector2.ZERO

			# Current max piston velocity
			var max_pst_vel: float = 0.0

			# For each cylinder
			for j in cyl_count:
				# Get cylinder
				var cyl: Cylinder = cylinders[j]

				# Get the crank angle relative to the piston
				var rel_crank_angle: float = fposmod(crank_angle + cyl.crank_offset, TAU * 2.0)

				# Get connection rod position
				var con_pos: Vector2 = Vector2(
					cos(rel_crank_angle + cyl.rotation) * crankshaft_connection_radius, 
					sin(rel_crank_angle + cyl.rotation) * crankshaft_connection_radius
				)

				# Get piston position
				var piston_pos: Vector2 = Vector2(
					cos(cyl.rotation) * cyl.piston_position,
					sin(cyl.rotation) * cyl.piston_position
				)

				# Get connection rod offset
				var con_off: Vector2 = con_pos - piston_pos

				# Get connection rod direction
				var con_dir: Vector2 = con_off.normalized()

				# Set the previous piston position
				cyl.piston_prev_position = cyl.piston_position

				# Set the piston position
				cyl.piston_position = piston_position(cyl.rod_length, crankshaft_connection_radius, rel_crank_angle)

				# Set the piston velocity
				cyl.piston_velocity = (cyl.piston_position - cyl.piston_prev_position) / dt

				# Will rev
				if not revving and crank_ang_vel >= max_crank_vel:
					revving = true
				# Will not rev
				if revving and crank_ang_vel <= max_crank_vel - margin_crank_vel:
					revving = false

				# Is accelerating
				if rel_crank_angle < PI:
					# Get acc
					var acc: float = gas
					if revving: acc *= 0.0

					# Apply acceleration
					crankshaft_impulse(con_pos, con_dir * piston_acceleration * dt * acc)

				# Apply drag
				if fposmod(rel_crank_angle, TAU) > PI:
					crankshaft_impulse(
						con_pos, 
						con_dir * cyl.piston_velocity * 
						# (engine_noise.get_noise_1d(time * engine_noise_freq) * 0.5 + 0.5) *
						clamp(piston_drag * dt, 0.0, 1.0)
					)
				
				# Add to overall velocity
				velocity += get_engine_point_velocity(
					Vector2(
						cos(cyl.rotation + engine_rot), 
						sin(cyl.rotation + engine_rot)
					) * (crankshaft_connection_radius + cyl.offset)
				) * cylf

			# Apply velocities
			crank_ang_vel += crank_applied_ang_vel
			engine_vel += engine_applied_vel
			engine_ang_vel += engine_applied_ang_vel

			# Get signal
			var signal: float = 0.0
			if abs(velocity.x) > abs(velocity.y):
				signal = velocity.x
			else:
				signal = velocity.y
			signal = velocity.x * engine_vibration_volume

			# Create frame
			var frame: Vector2 = Vector2.ONE * signal

			# Push frame to generator
			generator_playback.push_frame(frame)

			# Add frame to buffer
			buffer[buffer_pos] = frame
			buffer_pos = (buffer_pos + 1) % buffer_size

# Calculate piston position based on some variables
func piston_position(rod_len: float, crank_rad: float, crank_angle: float) -> float:
	var l: float = rod_len
	var r: float = crank_rad
	var a: float = crank_angle

	return cos(a) * r + sqrt(l * l - pow(sin(a) * r, 2.0))

# Get engine velocity at position
func get_engine_point_velocity(off: Vector2) -> Vector2:
	return engine_vel + Vector2(-engine_ang_vel * off.y, engine_ang_vel * off.x)

# Apply impulse to the crankshaft
func crankshaft_impulse(
	off: Vector2, j: Vector2
) -> void:
	crank_applied_ang_vel += off.cross(j) / crankshaft_connection_radius
	engine_vel += j
	engine_applied_ang_vel += off.cross(j) / crankshaft_connection_radius

# Called on draw
func _draw() -> void:
	# Transform by engine transform
	draw_set_transform(engine_pos * 100.0, engine_rot, Vector2.ONE)

	# Draw crankshaft
	draw_circle(
		Vector2.ZERO, 
		crankshaft_radius * 100.0,
		crankshaft_color
	)

	# Transform by crankshaft rotation
	draw_set_transform(engine_pos * 100.0, engine_rot + crank_angle, Vector2.ONE)

	# Draw a sticker showing crank position
	draw_rect(
		Rect2(crankshaft_radius * 100.0 - 16.0, -2.0, 16.0, 4.0),
		crankshaft_sticker_color
	)

	# Get number of cylinders
	var cyl_count: int = cylinders.size()

	# For each cylinder
	for i in cyl_count:
		# Get cylinder
		var cyl: Cylinder = cylinders[i]

		# Transform by cylinder rotation and offset
		draw_set_transform(engine_pos * 100.0, engine_rot + cyl.rotation, Vector2.ONE)

		# Get relative crank angle
		var rel_crank_angle: float = crank_angle + cyl.crank_offset

		# Draw cylinder walls
		draw_rect(
			Rect2(
				(crankshaft_connection_radius + cyl.offset) * 100.0,
				-cyl.width * 50.0 - 8.0,
				cyl.length * 100.0,
				8.0
			), cylinder_walls_color
		)
		draw_rect(
			Rect2(
				(crankshaft_connection_radius + cyl.offset) * 100.0,
				cyl.width * 50.0,
				cyl.length * 100.0,
				8.0
			), cylinder_walls_color
		)

		# Draw piston
		draw_rect(
			Rect2(
				cyl.piston_position * 100.0,
				-cyl.width * 50.0,
				cyl.piston_width * 100.0,
				cyl.width * 100.0
			), piston_color
		)

		# Draw connection rod
		draw_line(
			Vector2(cos(rel_crank_angle), sin(rel_crank_angle)) * crankshaft_connection_radius * 100.0,
			Vector2(cyl.piston_position * 100.0, 0.0), 
			connection_rod_color, 
			8.0
		)

		# Draw piston joint
		draw_circle(
			Vector2(cyl.piston_position * 100.0, 0.0),
			8.0, piston_joint_color
		)

		# Draw connection rod joint
		draw_circle(
			Vector2(cos(rel_crank_angle), sin(rel_crank_angle)) * crankshaft_connection_radius * 100.0,
			8.0, piston_joint_color
		)

	pass
