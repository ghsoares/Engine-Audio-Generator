extends CanvasLayer

# The simulation node
onready var simulation: Node = get_parent()

# The rpm label node
onready var rpm_label: Label = $VBox/RPM

# The gas slider
onready var gas_slider: HSlider = $VBox/Gas/Slider 

# The gas label
onready var gas_label: Label = $VBox/Gas/Value 

# The buffer draw node
var buffer_draw: RID

# Current rpm
var rpm: float

# Buffer draw size
export var buffer_draw_size: Vector2 = Vector2(512.0, 64.0)

# Buffer draw resolution
export var buffer_draw_resolution: int = 64

# Called when ready
func _ready() -> void:
	# Create the buffer draw node
	buffer_draw = VisualServer.canvas_item_create()

	# Set the parent
	VisualServer.canvas_item_set_parent(buffer_draw, self.get_canvas())

	# # Connect slider changed signal
	# gas_slider.connect("value_changed", self, "set_gas")

# Called every frame
func _process(delta: float) -> void:
	draw_buffer()

	# # Get the rpm
	# var rpm: float = simulation.crank_ang_vel / TAU * 60.0

	# self.rpm = lerp(self.rpm, rpm, clamp(4.0 * delta, 0.0, 1.0))
	
	# # Set the label text
	# rpm_label.text = "RPM: %d" % round(self.rpm)

	# # Set the slider value
	# gas_slider.set_block_signals(true)
	# gas_slider.value = simulation.gas
	# gas_slider.set_block_signals(false)

	# # Set the slider label
	# gas_label.text = "%.1f%%" % (simulation.gas * 100)

# # Called when the slider value chages
# func set_gas(val: float) -> void:
# 	simulation.gas = val

# Draw the buffer node
func draw_buffer() -> void:
	# Clear the draw node
	VisualServer.canvas_item_clear(buffer_draw)

	VisualServer.canvas_item_set_transform(
		buffer_draw,
		Transform2D.IDENTITY.translated(
			Vector2(
				8.0, 
				get_viewport().size.y - buffer_draw_size.y * 0.5 - 8.0
			)
		)
	)

	# Get buffer
	var buffer: PoolVector2Array = simulation.buffer

	# Get buffer size
	var buffer_size: int = simulation.buffer_size

	# Get buffer position
	var buffer_pos: int = simulation.buffer_pos

	# Polyline draw
	var polyline: PoolVector2Array
	polyline.resize(buffer_draw_resolution)
	
	# Draw buffer visualization
	for i in buffer_draw_resolution:
		# Get time
		var t: float = i / (buffer_draw_resolution - 1.0)

		# Get position
		var p: float = t * buffer_size

		# Get fract
		var f: float = p - floor(p)

		# Get first index
		var i0: int = int(p)
		i0 = clamp(i0, 0, buffer_size - 2)
		var i1: int = i0 + 1

		# Get frames
		var f1: Vector2 = buffer[(i0 + buffer_pos) % buffer_size]
		var f2: Vector2 = buffer[(i1 + buffer_pos) % buffer_size]

		# Get frame
		var frame: Vector2 = f1 * (1 - f) + f2 * f
		# frame.x = clamp(frame.x, -1.0, 1.0)

		# Get position
		var pos: Vector2 = Vector2(
			buffer_draw_size.x * t,
			frame.x * buffer_draw_size.y * 0.5
		)

		# Add to polyline
		polyline[i] = pos

		# # Get size
		# var size: Vector2 = Vector2(1, 1)

		# # Draw rect
		# VisualServer.canvas_item_add_rect(
		# 	buffer_draw,
		# 	Rect2(
		# 		pos - size * 0.5, size
		# 	), Color(0, 1, 0, 1.0)
		# )
	
	# Draw boundaries
	VisualServer.canvas_item_add_rect(
		buffer_draw, Rect2(
			-0.0, -buffer_draw_size.y * 0.5,
			buffer_draw_size.x, buffer_draw_size.y
		), Color(1.0, 1.0, 1.0, 0.1)
	)

	# Draw middle line
	VisualServer.canvas_item_add_line(
		buffer_draw, Vector2(0.0, 0.0),
		Vector2(buffer_draw_size.x, 0.0), Color(1.0, 1.0, 1.0, 0.25)
	)

	# Draw the polyline
	VisualServer.canvas_item_add_polyline(
		buffer_draw, polyline, PoolColorArray([Color.green])
	)

	
