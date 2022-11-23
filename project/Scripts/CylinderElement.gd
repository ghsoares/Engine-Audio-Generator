extends Control

onready var id_label: Label = $ID
onready var delete_button: Button = $Delete
onready var crank_offset_slider: TextSlider = $VBox/CrankOffset
onready var piston_motion_factor_slider: TextSlider = $VBox/PistonMotionFactor
onready var ignition_factor_slider: TextSlider = $VBox/IgnitionFactor
onready var ignition_time_slider: TextSlider = $VBox/IgnitionTime
onready var intake_pipe_length_slider: TextSlider = $VBox/IntakePipeLength
onready var exhaust_pipe_length_slider: TextSlider = $VBox/ExhaustPipeLength
onready var extractor_pipe_length_slider: TextSlider = $VBox/ExtractorPipeLength

var id: int setget set_id
var crank_offset: float setget set_crank_offset
var piston_motion_factor: float setget set_piston_motion_factor
var ignition_factor: float setget set_ignition_factor
var ignition_time: float setget set_ignition_time
var intake_pipe_length: float setget set_intake_pipe_length
var exhaust_pipe_length: float setget set_exhaust_pipe_length
var extractor_pipe_length: float setget set_extractor_pipe_length

signal changed(element)
signal deleted(element)

func _ready() -> void:
	delete_button.connect("pressed", self, "on_delete_pressed")
	crank_offset_slider.connect("value_changed", self, "set_crank_offset")
	piston_motion_factor_slider.connect("value_changed", self, "set_piston_motion_factor")
	ignition_factor_slider.connect("value_changed", self, "set_ignition_factor")
	ignition_time_slider.connect("value_changed", self, "set_ignition_time")
	intake_pipe_length_slider.connect("value_changed", self, "set_intake_pipe_length")
	exhaust_pipe_length_slider.connect("value_changed", self, "set_exhaust_pipe_length")
	extractor_pipe_length_slider.connect("value_changed", self, "set_extractor_pipe_length")

func set_id(val: int) -> void:
	id = val
	id_label.text = "%d." % (id + 1)

func on_delete_pressed() -> void:
	emit_signal("deleted", self)

func set_crank_offset(val: float) -> void:
	crank_offset = val

	crank_offset_slider.set_block_signals(true)

	crank_offset_slider.value = crank_offset
	crank_offset_slider.text = "Crank Offset: %.2f Cycles" % crank_offset
	emit_signal("changed", self)

	crank_offset_slider.set_block_signals(false)

func set_piston_motion_factor(val: float) -> void:
	piston_motion_factor = val

	piston_motion_factor_slider.set_block_signals(true)

	piston_motion_factor_slider.value = piston_motion_factor
	piston_motion_factor_slider.text = "Piston Motion Factor: %.2f" % piston_motion_factor
	emit_signal("changed", self)

	piston_motion_factor_slider.set_block_signals(false)

func set_ignition_factor(val: float) -> void:
	ignition_factor = val

	ignition_factor_slider.set_block_signals(true)

	ignition_factor_slider.value = ignition_factor
	ignition_factor_slider.text = "Ignition Factor: %.2f" % ignition_factor
	emit_signal("changed", self)

	ignition_factor_slider.set_block_signals(false)

func set_ignition_time(val: float) -> void:
	ignition_time = val

	ignition_time_slider.set_block_signals(true)

	ignition_time_slider.value = ignition_time
	ignition_time_slider.text = "Ignition Time: %.2f" % ignition_time
	emit_signal("changed", self)

	ignition_time_slider.set_block_signals(false)

func set_intake_pipe_length(val: float) -> void:
	intake_pipe_length = val

	intake_pipe_length_slider.set_block_signals(true)

	intake_pipe_length_slider.value = intake_pipe_length
	intake_pipe_length_slider.text = "Intake Pipe Length: %.2f Meters" % intake_pipe_length
	emit_signal("changed", self)

	intake_pipe_length_slider.set_block_signals(false)

func set_exhaust_pipe_length(val: float) -> void:
	exhaust_pipe_length = val

	exhaust_pipe_length_slider.set_block_signals(true)

	exhaust_pipe_length_slider.value = exhaust_pipe_length
	exhaust_pipe_length_slider.text = "Exhaust Pipe Length: %.2f Meters" % exhaust_pipe_length
	emit_signal("changed", self)

	exhaust_pipe_length_slider.set_block_signals(false)

func set_extractor_pipe_length(val: float) -> void:
	extractor_pipe_length = val

	extractor_pipe_length_slider.set_block_signals(true)

	extractor_pipe_length_slider.value = extractor_pipe_length
	extractor_pipe_length_slider.text = "Extractor Pipe Length: %.2f Meters" % extractor_pipe_length
	emit_signal("changed", self)

	extractor_pipe_length_slider.set_block_signals(false)















