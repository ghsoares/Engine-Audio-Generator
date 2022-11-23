tool
extends Control
class_name TextSlider

# Is dragging
var dragging: bool 

# Input line
var input_line: LineEdit

# Should debug
var debug: bool

# Display text
export var text: String = "Insert text here" setget set_text

# # The font
# export var font: Font setget set_font

# Min value
export var min_value: float = 0.0 setget set_min_value

# Max value
export var max_value: float = 100.0 setget set_max_value

# Current value
export var value: float = 50.0 setget set_value

# Step
export var step: float = 0.1 setget set_step

# # Margin
# export var text_margin: Vector2 = Vector2(8.0, 4.0) setget set_text_margin

# # Background color
# export var background_color: Color = Color.gray setget set_background_color

# # Fill color
# export var fill_color: Color = Color.white setget set_fill_color

signal changed
signal value_changed(new_val)

# Initializes
func _init() -> void:
	input_line = LineEdit.new()
	input_line.anchor_left = 0
	input_line.anchor_top = 0
	input_line.anchor_right = 1
	input_line.anchor_bottom = 1

	input_line.connect("text_entered", self, "input_text_entered")
	input_line.connect("focus_exited", self, "close_input_line")
	input_line.connect("gui_input", self, "input_line_input")

	add_child(input_line)
	input_line.hide()

# Gui input
func _gui_input(event: InputEvent) -> void:
	if event is InputEventMouse:
		# Get position
		var pos: Vector2 = event.position

		# Is inside control
		var inside: bool = pos.x >= 0 and pos.x <= rect_size.x and pos.y >= 0 and pos.y <= rect_size.y

		# Update value
		var update_value: bool = false
		var new_value: float = 0.0

		if event is InputEventMouseButton:
			if event.button_index == BUTTON_LEFT:
				if event.pressed and inside:
					dragging = true
					update_value = true
					new_value = pos.x / rect_size.x
				elif not event.pressed:
					dragging = false
			if event.button_index == BUTTON_RIGHT:
				if event.pressed and inside:
					open_input_line()
		elif event is InputEventMouseMotion:
			if dragging:
				update_value = true
				new_value = pos.x / rect_size.x
		
		if update_value:
			new_value = clamp(new_value, 0, 1)
			set_value(min_value + (max_value - min_value) * new_value)
	
# On input line input
func input_line_input(event: InputEvent) -> void:
	if event is InputEventKey:
		if event.scancode == KEY_ESCAPE:
			if event.pressed:
				if input_line.has_focus():
					close_input_line()

# Get the minimum size
func _get_minimum_size() -> Vector2:
	# Get margin constants
	var margin_x: float = 4.0
	var margin_y: float = 4.0
	if has_constant("margin_x", "TextSlider"):
		margin_x = get_constant("margin_x", "TextSlider")
	if has_constant("margin_y", "TextSlider"):
		margin_y = get_constant("margin_y", "TextSlider")

	# Get font
	var font: Font = get_font("font", "Label")
	if has_font("font", "TextSlider"):
		font = get_font("font", "TextSlider")
	
	return font.get_string_size(text) + Vector2(margin_x, margin_y) * 2

# Draw the control
func _draw() -> void:
	var percentage: float = clamp((value - min_value) / (max_value - min_value), 0.0, 1.0)

	# Get fill and background styles
	var fill_style: StyleBox
	var background_style: StyleBox
	if has_stylebox("fill_style", "TextSlider"):
		fill_style = get_stylebox("fill_style", "TextSlider")
	else:
		fill_style = StyleBoxFlat.new()
		fill_style.bg_color = Color(0.1, 0.05, 0.12)
		
	if has_stylebox("background_style", "TextSlider"):
		background_style = get_stylebox("background_style", "TextSlider")
	else:
		background_style = StyleBoxFlat.new()
		background_style.bg_color = Color(0.1, 0.05, 0.12, 0.25)

	# Get margin constants
	var margin_x: float = 4.0
	var margin_y: float = 4.0
	if has_constant("margin_x", "TextSlider"):
		margin_x = get_constant("margin_x", "TextSlider")
	if has_constant("margin_y", "TextSlider"):
		margin_y = get_constant("margin_y", "TextSlider")

	# Get font
	var font: Font = get_font("font", "Label")
	if has_font("font", "TextSlider"):
		font = get_font("font", "TextSlider")

	# Draw background
	if percentage < 1.0:
		background_style.draw(
			get_canvas_item(), Rect2(
				0.0, 0.0,
				rect_size.x, rect_size.y
			)
		)

	# Draw fill
	if percentage > 0.0:
		fill_style.draw(
			get_canvas_item(), Rect2(
				0.0, 0.0,
				rect_size.x * percentage, rect_size.y
			)
		)
	
	# Draw text
	draw_string(font, Vector2(margin_x, font.get_ascent() + margin_y), text)

# Input line entered
func input_text_entered(text: String) -> void:
	# Create a expression
	var expr: Expression = Expression.new()
	
	# Parse the expression
	var parse_err: int = expr.parse(text)
	if parse_err == OK:
		# Execute and evaluate to number
		var val = expr.execute()
		if val is float or val is int:
			set_value(val)
	
	close_input_line()

# Open input line
func open_input_line() -> void:
	input_line.set_block_signals(true)

	input_line.text = str(value)
	input_line.show()
	input_line.grab_focus()

	input_line.set_block_signals(false)

# Close input line
func close_input_line() -> void:
	input_line.set_block_signals(true)

	input_line.release_focus()
	input_line.hide()

	input_line.set_block_signals(false)

# Setters
func set_text(val: String) -> void:
	if val != text:
		text = val
		emit_signal("changed")
		minimum_size_changed()
		update()

# func set_font(val: Font) -> void:
# 	if val != font:
# 		font = val
# 		emit_signal("changed")
# 		minimum_size_changed()
# 		update()

func set_min_value(val: float) -> void:
	if val != min_value:
		min_value = val
		emit_signal("changed")
		update()

func set_max_value(val: float) -> void:
	if val != max_value:
		max_value = val
		emit_signal("changed")
		update()

func set_value(val: float) -> void:
	if val != value:
		if step > 0.0:
			val = stepify(val, step)
		value = val
		emit_signal("value_changed", value)
		emit_signal("changed")
		update()

func set_step(val: float) -> void:
	if val != step:
		step = val
		emit_signal("changed")
		update()

# func set_text_margin(val: Vector2) -> void:
# 	if val != text_margin:
# 		text_margin = val
# 		minimum_size_changed()
# 		emit_signal("changed")
# 		update()

# func set_background_color(val: Color) -> void:
# 	if val != background_color:
# 		background_color = val
# 		emit_signal("changed")
# 		update()
		
# func set_fill_color(val: Color) -> void:
# 	if val != fill_color:
# 		fill_color = val
# 		emit_signal("changed")
# 		update()





