extends Control

# The audio player
onready var player: AudioStreamPlayer = $Player

# The audio stream
var stream: AudioStreamGenerator

# The audio playback
var playback: AudioStreamGeneratorPlayback

# Current engine config
var engine_config: EngineConfig

# Generator
var generator: EngineAudioGenerator

# Is updating ui
var ui_updating: bool

# Current generation time
var time: float = 0.0

# Current opened dialogs
var opened_dialogs: int = 0

var muffler_element_scene: PackedScene = load("res://Scenes/MufflerElement.tscn")
var cylinder_element_scene: PackedScene = load("res://Scenes/CylinderElement.tscn")

# Interface nodes
onready var interface_root: Control = $UI/VBox/Engine/Panel/VBox
onready var popups_root: Control = $Popups
onready var options_root: Control = interface_root.get_node("Options/VBox")
onready var mix_root: Control = interface_root.get_node("Mix/VBox")
onready var engine_params_root: Control = interface_root.get_node("EngineParams/VBox")
onready var muffler_params_root: Control = interface_root.get_node("MufflerParams/VBox")
onready var cylinder_params_root: Control = interface_root.get_node("CylinderParams/VBox")
onready var interface: Dictionary = {
	# Options
	"reset_config": options_root.get_node("ResetConfig"),
	"save_config": options_root.get_node("SaveConfig"),
	"load_config": options_root.get_node("LoadConfig"),
	"reset_buffer": options_root.get_node("ResetBuffer"),
	"export": options_root.get_node("Export"),

	# Mix
	"engine_rpm": mix_root.get_node("RPM"),
	"master_volume": mix_root.get_node("MasterVolume"),
	"intake_volume": mix_root.get_node("IntakeVolume"),
	"exhaust_volume": mix_root.get_node("ExhaustVolume"),
	"vibrations_volume": mix_root.get_node("VibrationsVolume"),
	"lowpass_freq": mix_root.get_node("LowPassFreq"),
	"sample_rate": mix_root.get_node("SampleRate"),

	# Engine params
	"vibrations_lowpass_freq": engine_params_root.get_node("VibrationsLowPassFreq"),
	"intake_noise_factor": engine_params_root.get_node("IntakeNoiseFactor"),
	"intake_noise_frequency": engine_params_root.get_node("IntakeNoiseFrequency"),
	"intake_noise_lowpass_freq": engine_params_root.get_node("IntakeNoiseLowPassFreq"),
	"intake_valve_shift": engine_params_root.get_node("IntakeValveShift"),
	"exhaust_valve_shift": engine_params_root.get_node("ExhaustValveShift"),
	"crankshaft_fluctuation_factor": engine_params_root.get_node("CrankshaftFluctuationFactor"),
	"crankshaft_fluctuation_frequency": engine_params_root.get_node("CrankshaftFluctuationFrequency"),
	"crankshaft_fluctuation_lowpass_freq": engine_params_root.get_node("CrankshaftFluctuationLowPassFreq"),

	# Muffler params
	"straight_pipe_extractor_refl": muffler_params_root.get_node("StraightPipeExtractorRefl"),
	"straight_pipe_muffler_refl": muffler_params_root.get_node("StraightPipeMufflerRefl"),
	"straight_pipe_length": muffler_params_root.get_node("StraightPipeLength"),
	"muffler_exhaust_refl": muffler_params_root.get_node("MufflerElementsOutputRefl"),
	"muffler_elements": muffler_params_root.get_node("MufflerElements"),
	"add_muffler_element": muffler_params_root.get_node("AddMufflerElement"),

	# Cylinder params
	"open_intake_refl": cylinder_params_root.get_node("OpenedIntakeValveRefl"),
	"closed_intake_refl": cylinder_params_root.get_node("ClosedIntakeValveRefl"),
	"open_exhaust_refl": cylinder_params_root.get_node("OpenedExhaustValveRefl"),
	"closed_exhaust_refl": cylinder_params_root.get_node("ClosedExhaustValveRefl"),
	"intake_open_end_refl": cylinder_params_root.get_node("IntakeOpenEndRefl"),
	"extractor_open_end_refl": cylinder_params_root.get_node("ExtractorOpenEndRefl"),
	"cylinder_elements": cylinder_params_root.get_node("CylinderElements"),
	"add_cylinder_element": cylinder_params_root.get_node("AddCylinderElement")
}
onready var popups: Dictionary = {
	"save_config": popups_root.get_node("SaveConfig"),
	"load_config": popups_root.get_node("LoadConfig"),
	"message_dialog": popups_root.get_node("MessageDialog"),
	"confirmation_dialog": popups_root.get_node("ConfirmationDialog"),
}

# Called on ready
func _ready() -> void:
	generator = EngineAudioGenerator.new()

	stream = player.stream
	playback = player.get_stream_playback()

	generator.stream = stream
	generator.playback = playback

	player.play()

	popups_root.hide()

	reset_engine_config()

# Called every frame
func _process(delta: float) -> void:
	stream.mix_rate = engine_config.sample_rate
	
	# Get generation spacing
	var sp: float = 1.0 / 60.0
	var latency: float = 20.0 / 1000.0
	
	# Get frames spacing
	var frames_sp: int = floor(engine_config.sample_rate * sp)
	
	# Get spacing
	sp = frames_sp / float(engine_config.sample_rate)
	
	# Get playback position
	var pos: float = player.get_playback_position()
	
	# Generate by spacing
	while time <= pos + latency:
		generator.fill_buffer(frames_sp)

		time += sp

# Update UI variables according to the engine config
func update_ui() -> void:
	if ui_updating: return

	ui_updating = true

	interface.crankshaft_fluctuation_factor.debug = true

	# Options
	if not interface.reset_config.is_connected("pressed", self, "open_reset_engine_config"):
		interface.reset_config.connect("pressed", self, "open_reset_engine_config")
	if not interface.save_config.is_connected("pressed", self, "open_save_engine_config"):
		interface.save_config.connect("pressed", self, "open_save_engine_config")
	if not interface.load_config.is_connected("pressed", self, "open_load_engine_config"):
		interface.load_config.connect("pressed", self, "open_load_engine_config")
	if not interface.reset_buffer.is_connected("pressed", self, "reset_buffer"):
		interface.reset_buffer.connect("pressed", self, "reset_buffer")
	if not interface.export.is_connected("pressed", self, "export"):
		interface.export.connect("pressed", self, "export")

	# Mix
	update_slider_variable(
		engine_config, interface.engine_rpm, "rpm",
		"Engine RPM: %.2f (%.2f hz)",
		engine_config.rpm, [engine_config.rpm, engine_config.rpm / 60.0]
	)
	update_slider_variable(
		engine_config, interface.master_volume, "volume",
		"Master Volume: %.2f%%",
		engine_config.volume, [engine_config.volume * 100]
	)
	update_slider_variable(
		engine_config, interface.intake_volume, "intake_volume",
		"Intake Volume: %.2f%%",
		engine_config.intake_volume, [engine_config.intake_volume * 100]
	)
	update_slider_variable(
		engine_config, interface.exhaust_volume, "exhaust_volume",
		"Exhaust Volume: %.2f%%",
		engine_config.exhaust_volume, [engine_config.exhaust_volume * 100]
	)
	update_slider_variable(
		engine_config, interface.vibrations_volume, "vibrations_volume",
		"Vibrations Volume: %.2f%%",
		engine_config.vibrations_volume, [engine_config.vibrations_volume * 100]
	)
	update_slider_variable(
		engine_config, interface.lowpass_freq, "dc_filter_frequency",
		"Lowpass-Filter Frequency: %.2f HZ",
		engine_config.dc_filter_frequency, [engine_config.dc_filter_frequency]
	)
	update_slider_variable(
		engine_config, interface.sample_rate, "sample_rate",
		"Sample Rate: %.2f HZ",
		engine_config.sample_rate, [engine_config.sample_rate]
	)

	# Engine params
	update_slider_variable(
		engine_config, interface.vibrations_lowpass_freq, "vibrations_filter_frequency",
		"Vibrations Lowpass-Filter Frequency: %.2f HZ",
		engine_config.vibrations_filter_frequency, [engine_config.vibrations_filter_frequency]
	)
	update_slider_variable(
		engine_config, interface.intake_noise_factor, "intake_noise_factor",
		"Intake Noise Factor: %.2f",
		engine_config.intake_noise_factor, [engine_config.intake_noise_factor]
	)
	update_slider_variable(
		engine_config, interface.intake_noise_frequency, "intake_noise_frequency",
		"Intake Noise Frequency: %.2f HZ",
		engine_config.intake_noise_frequency, [engine_config.intake_noise_frequency]
	)
	update_slider_variable(
		engine_config, interface.intake_noise_lowpass_freq, "intake_noise_filter_frequency",
		"Intake Noise Lowpass-Filter Frequency: %.2f HZ",
		engine_config.intake_noise_filter_frequency, [engine_config.intake_noise_filter_frequency]
	)
	update_slider_variable(
		engine_config, interface.intake_valve_shift, "intake_valve_shift",
		"Intake Valve Cam Shift: %.2f Cycles",
		engine_config.intake_valve_shift, [engine_config.intake_valve_shift]
	)
	update_slider_variable(
		engine_config, interface.exhaust_valve_shift, "exhaust_valve_shift",
		"Exhaust Valve Cam Shift: %.2f Cycles",
		engine_config.exhaust_valve_shift, [engine_config.exhaust_valve_shift]
	)
	update_slider_variable(
		engine_config, interface.crankshaft_fluctuation_factor, "crankshaft_fluctuation",
		"Crankshaft Fluctuation Factor: %.2f",
		engine_config.crankshaft_fluctuation, [engine_config.crankshaft_fluctuation]
	)
	update_slider_variable(
		engine_config, interface.crankshaft_fluctuation_frequency, "crankshaft_fluctuation_frequency",
		"Crankshaft Fluctuation Frequency: %.2f HZ",
		engine_config.crankshaft_fluctuation_frequency, [engine_config.crankshaft_fluctuation_frequency]
	)
	update_slider_variable(
		engine_config, interface.crankshaft_fluctuation_lowpass_freq, "crankshaft_fluctuation_filter_frequency",
		"Crankshaft Fluctuation Lowpass-Filter Frequency: %.2f HZ",
		engine_config.crankshaft_fluctuation_filter_frequency, [engine_config.crankshaft_fluctuation_filter_frequency]
	)
	
	# Muffler parameters
	update_slider_variable(
		engine_config, interface.straight_pipe_extractor_refl, "straight_pipe_extractor_side_refl",
		"Straight Pipe Extractor-Side Reflectivity: %.2f",
		engine_config.straight_pipe_extractor_side_refl, [engine_config.straight_pipe_extractor_side_refl]
	)
	update_slider_variable(
		engine_config, interface.straight_pipe_muffler_refl, "straight_pipe_muffler_side_refl",
		"Straight Pipe Muffler-Side Reflectivity: %.2f",
		engine_config.straight_pipe_muffler_side_refl, [engine_config.straight_pipe_muffler_side_refl]
	)
	update_slider_variable(
		engine_config, interface.straight_pipe_length, "straight_pipe_length",
		"Straight Pipe Length: %.2f Meters",
		engine_config.straight_pipe_length, [engine_config.straight_pipe_length]
	)
	update_slider_variable(
		engine_config, interface.muffler_exhaust_refl, "output_side_refl",
		"Muffler Elements Exhaust-Side Reflectivity: %.2f",
		engine_config.output_side_refl, [engine_config.output_side_refl]
	)

	var new_muffler_elements: Array = engine_config.muffler_elements_output

	if new_muffler_elements.size() < interface.muffler_elements.get_child_count():
		var count: int = interface.muffler_elements.get_child_count()
		var idx: int = count - 1
		while idx >= new_muffler_elements.size():
			var c: Control = interface.muffler_elements.get_child(idx)
			interface.muffler_elements.remove_child(c)
			c.queue_free()
			idx -= 1
	elif new_muffler_elements.size() > interface.muffler_elements.get_child_count():
		var count: int = new_muffler_elements.size()
		var idx: int = interface.muffler_elements.get_child_count()
		while idx < count:
			var c: Control = muffler_element_scene.instance()
			interface.muffler_elements.add_child(c)

			c.connect("changed", self, "on_muffler_element_changed")
			c.connect("deleted", self, "on_muffler_element_deleted")

			idx += 1

	for i in range(new_muffler_elements.size()):
		var c: Control = interface.muffler_elements.get_child(i)

		c.set_block_signals(true)

		c.id = i
		c.cavity_length = new_muffler_elements[i].cavity_length

		c.set_block_signals(false)

	if not interface.add_muffler_element.is_connected("pressed", self, "add_muffler_element"):
		interface.add_muffler_element.connect("pressed", self, "add_muffler_element")

	# Cylinder params
	update_slider_variable(
		engine_config, interface.open_intake_refl, "cylinder_intake_opened_refl",
		"Opened Intake Valve Cavity Reflectivity: %.2f",
		engine_config.cylinder_intake_opened_refl, [engine_config.cylinder_intake_opened_refl]
	)
	update_slider_variable(
		engine_config, interface.closed_intake_refl, "cylinder_intake_closed_refl",
		"Closed Intake Valve Cavity Reflectivity: %.2f",
		engine_config.cylinder_intake_closed_refl, [engine_config.cylinder_intake_closed_refl]
	)
	update_slider_variable(
		engine_config, interface.open_exhaust_refl, "cylinder_exhaust_opened_refl",
		"Opened Exhaust Valve Cavity Reflectivity: %.2f",
		engine_config.cylinder_exhaust_opened_refl, [engine_config.cylinder_exhaust_opened_refl]
	)
	update_slider_variable(
		engine_config, interface.closed_exhaust_refl, "cylinder_exhaust_closed_refl",
		"Closed Exhaust Valve Cavity Reflectivity: %.2f",
		engine_config.cylinder_exhaust_closed_refl, [engine_config.cylinder_exhaust_closed_refl]
	)
	update_slider_variable(
		engine_config, interface.intake_open_end_refl, "cylinder_intake_open_end_refl",
		"Intake-Cavity Open End Reflectivity: %.2f",
		engine_config.cylinder_intake_open_end_refl, [engine_config.cylinder_intake_open_end_refl]
	)
	update_slider_variable(
		engine_config, interface.extractor_open_end_refl, "cylinder_extractor_open_end_refl",
		"Extractor-Cavity Open End Reflectivity: %.2f",
		engine_config.cylinder_extractor_open_end_refl, [engine_config.cylinder_extractor_open_end_refl]
	)

	var new_cylinder_elements: Array = engine_config.cylinder_elements

	if new_cylinder_elements.size() < interface.cylinder_elements.get_child_count():
		var count: int = interface.cylinder_elements.get_child_count()
		var idx: int = count - 1
		while idx >= new_cylinder_elements.size():
			var c: Control = interface.cylinder_elements.get_child(idx)
			interface.cylinder_elements.remove_child(c)
			c.queue_free()
			idx -= 1
	elif new_cylinder_elements.size() > interface.cylinder_elements.get_child_count():
		var count: int = new_cylinder_elements.size()
		var idx: int = interface.cylinder_elements.get_child_count()
		while idx < count:
			var c: Control = cylinder_element_scene.instance()
			interface.cylinder_elements.add_child(c)

			c.connect("changed", self, "on_cylinder_element_changed")
			c.connect("deleted", self, "on_cylinder_element_deleted")

			idx += 1
	
	for i in range(new_cylinder_elements.size()):
		var c: Control = interface.cylinder_elements.get_child(i)

		c.set_block_signals(true)

		c.id = i
		c.crank_offset = new_cylinder_elements[i].crank_offset
		c.piston_motion_factor = new_cylinder_elements[i].piston_motion_factor
		c.ignition_factor = new_cylinder_elements[i].ignition_factor
		c.ignition_time = new_cylinder_elements[i].ignition_time
		c.intake_pipe_length = new_cylinder_elements[i].intake_pipe_length
		c.exhaust_pipe_length = new_cylinder_elements[i].exhaust_pipe_length
		c.extractor_pipe_length = new_cylinder_elements[i].extractor_pipe_length

		c.set_block_signals(false)

	if not interface.add_cylinder_element.is_connected("pressed", self, "add_cylinder_element"):
		interface.add_cylinder_element.connect("pressed", self, "add_cylinder_element")

	ui_updating = false

# Update a single slider variable
func update_slider_variable(
	model, slider: Node,
	model_name: String, view_text: String,
	value, view_format_text: Array = []
) -> void:
	slider.set_block_signals(true)
	
	model.set(model_name, value)
	slider.value = value
	slider.text = view_text % view_format_text

	slider.set_block_signals(false)

	if slider.is_connected("value_changed", self, "on_slider_update"):
		slider.disconnect("value_changed", self, "on_slider_update")
	slider.connect("value_changed", self, "on_slider_update", [model, slider, model_name, view_text, view_format_text])

	update_ui()

# Called when a slider changes
func on_slider_update(value, model, slider: Node, model_name: String, view_text: String, view_format_text: Array) -> void:
	update_slider_variable(model, slider, model_name, view_text, value, view_format_text)

# Called when a muffler changes
func on_muffler_element_changed(muf) -> void:
	# Get mufflers
	var mufflers: Array = engine_config.muffler_elements_output

	# Get muffler
	var muffler = mufflers[muf.id]

	# Set variables
	muffler.cavity_length = muf.cavity_length

	# Set mufflers back
	engine_config.muffler_elements_output = mufflers

	update_ui()

# Called when a muffler is deleted
func on_muffler_element_deleted(muf) -> void:
	# Get mufflers
	var mufflers: Array = engine_config.muffler_elements_output
	if mufflers.size() <= 1: return

	# Delete muffler
	mufflers.remove(muf.id)

	# Set mufflers back
	engine_config.muffler_elements_output = mufflers

	update_ui()

# Called when a cylinder changes
func on_cylinder_element_changed(cyl) -> void:
	# Get cylinders
	var cylinders: Array = engine_config.cylinder_elements

	# Get cylinder
	var cylinder = cylinders[cyl.id]

	# Set variables
	cylinder.crank_offset = cyl.crank_offset
	cylinder.piston_motion_factor = cyl.piston_motion_factor
	cylinder.ignition_factor = cyl.ignition_factor
	cylinder.ignition_time = cyl.ignition_time
	cylinder.intake_pipe_length = cyl.intake_pipe_length
	cylinder.exhaust_pipe_length = cyl.exhaust_pipe_length
	cylinder.extractor_pipe_length = cyl.extractor_pipe_length

	# Set cylinders back
	engine_config.cylinder_elements = cylinders

	update_ui()

# Called when a cylinder is deleted
func on_cylinder_element_deleted(cyl) -> void:
	# Get cylinders
	var cylinders: Array = engine_config.cylinder_elements
	if cylinders.size() <= 1: return

	# Delete cylinder
	cylinders.remove(cyl.id)

	# Set cylinders back
	engine_config.cylinder_elements = cylinders

	update_ui()

# Called to add a muffler element
func add_muffler_element() -> void:
	# Create muffler
	var muffler

	# Get mufflers
	var mufflers: Array = engine_config.muffler_elements_output

	# Duplicate last or create one
	if mufflers.size() > 0:
		muffler = mufflers[mufflers.size() - 1].duplicate()
	else:
		muffler = EngineMufflerConfig.new()
	
	# Append new muffler
	mufflers.append(muffler)

	# Set engine config mufflers
	engine_config.muffler_elements_output = mufflers

	# Update ui
	update_ui()

# Called to add a cylinder element
func add_cylinder_element() -> void:
	# Create cylinder
	var cylinder

	# Get cylinders
	var cylinders: Array = engine_config.cylinder_elements

	# Duplicate last or create one
	if cylinders.size() > 0:
		cylinder = cylinders[cylinders.size() - 1].duplicate()
	else:
		cylinder = EngineCylinderConfig.new()
	
	# Append new cylinder
	cylinders.append(cylinder)

	# Set engine config cylinders
	engine_config.cylinder_elements = cylinders

	# Update ui
	update_ui()

# Called to open reset engine dialog
func open_reset_engine_config() -> void:
	# Show popups container
	popups_root.show()

	# Open confirmation dialog
	if not popups.confirmation_dialog.visible:
		popups.confirmation_dialog.window_title = "Reset Engine Config"
		popups.confirmation_dialog.dialog_text = "Are you sure you want to reset the configuration?"

		popups.confirmation_dialog.popup()
		opened_dialogs += 1

		# Connect signals
		popups.confirmation_dialog.connect("confirmed", self, "reset_engine_config")
		popups.confirmation_dialog.connect("popup_hide", self, "close_reset_engine_config")

# Called to open save engine config
func open_save_engine_config() -> void:
	# Show popups container
	popups_root.show()

	# Open load dialog
	if not popups.save_config.visible:
		if engine_config.resource_path != "":
			popups.save_config.current_path = engine_config.resource_path
		popups.save_config.invalidate()
		popups.save_config.popup()
		opened_dialogs += 1

		# Connect signals
		popups.save_config.connect("file_selected", self, "save_engine_config")
		popups.save_config.connect("popup_hide", self, "close_save_engine_config")

# Called to open load engine config
func open_load_engine_config() -> void:
	# Show popups container
	popups_root.show()

	# Open load dialog
	if not popups.load_config.visible:
		if engine_config.resource_path != "":
			popups.load_config.current_path = engine_config.resource_path
		popups.load_config.invalidate()
		popups.load_config.popup()
		opened_dialogs += 1

		# Connect signals
		popups.load_config.connect("file_selected", self, "load_engine_config")
		popups.load_config.connect("popup_hide", self, "close_load_engine_config")

# Called to open message
func open_message_dialog(title: String, message: String) -> void:
	# Open message dialog
	popups.message_dialog.window_title = title
	popups.message_dialog.dialog_text = message
	if not popups.message_dialog.visible:
		popups.message_dialog.popup()
		opened_dialogs += 1
		popups_root.show()

		# Connect signals
		popups.message_dialog.connect("popup_hide", self, "close_message_dialog")

# Called to reset the engine config
func reset_engine_config() -> void:
	player.stop()
	
	# Create a new config
	engine_config = EngineConfig.new()
	engine_config.sample_rate = 44100
	engine_config.clear_buffer()
	time = 0
	player.play()
	
	# Set the generator config
	generator.engine_configuration = engine_config
	update_ui()

	if popups.confirmation_dialog.visible:
		close_reset_engine_config()

# Called to reset audio buffer
func reset_buffer() -> void:
	player.stop()
	engine_config.clear_buffer()
	time = 0
	player.play()

# Called to export audio
func export() -> void:
	var recorder = EngineAudioRecorder.new()

	recorder.min_rpm = 1000
	recorder.top_rpm = 12000
	recorder.sample_count = 8
	recorder.padding_frames = 64
	recorder.fade_time = 0.1
	recorder.preheat_time = 1
	recorder.duration_per_sample = 2.0
	recorder.engine_configuration = engine_config
	recorder.include_audio_header = true

	recorder.record()
	var save_path: String = "res://Outputs/Cyl8v10/"
	var crankshaft_audio: AudioStreamSample = recorder.get_crankshaft_recording()
	var ignition_audio: AudioStreamSample = recorder.get_ignition_recording()
	var exhaust_audio: AudioStreamSample = recorder.get_exhaust_recording()
	print("Crankshaft save status: %d" % crankshaft_audio.save_to_wav(save_path + "Crankshaft.wav"))
	print("Ignition save status: %d" % ignition_audio.save_to_wav(save_path + "Ignition.wav"))
	print("Exhaust save status: %d" % exhaust_audio.save_to_wav(save_path + "Exhaust.wav"))

# Called to save the engine config
func save_engine_config(path: String) -> void:
	# Why GDNATIVE resources aren't recognized as resources??????
	var res = engine_config

	# Get the engine name from the file name
	var engine_name: String = path.get_file().trim_suffix("." + path.get_extension())

	# Set the engine name
	res.resource_name = engine_name

	# Set the engine path
	res.resource_path = path

	# Save as resource
	var err: int = ResourceSaver.save(path, res)

	# Invalid
	if err != OK:
		open_message_dialog("Error saving config file", "An error occurred when trying to save the config file at %s.\nError status: %d.\nPlease contact the developer if the error persist." % [path, err])
		return

# Called to load the engine config
func load_engine_config(path: String) -> void:
	# Load resource
	var res = ResourceLoader.load(path, "", true)
	
	# Error occurred
	if res == null:
		open_message_dialog("Error loading config file", "An error occurred when trying to load the config file at %s.\nPlease contact the developer if the error persist." % [path])
		return

	# Get the engine name from the file name
	var engine_name: String = path.get_file().trim_suffix("." + path.get_extension())

	# Loaded, set the engine config and close the dialog
	engine_config = res
	engine_config.resource_name = engine_name
	engine_config.resource_path = path
	engine_config.clear_buffer()
	generator.engine_configuration = engine_config
	update_ui()

# Called to close reset engine config
func close_reset_engine_config() -> void:
	# Connect signals
	popups.confirmation_dialog.disconnect("confirmed", self, "reset_engine_config")
	popups.confirmation_dialog.disconnect("popup_hide", self, "close_reset_engine_config")

	# Hide confirmation dialog
	popups.confirmation_dialog.hide()

	opened_dialogs -= 1
	if opened_dialogs == 0:
		popups_root.hide()

# Called to close save engine config
func close_save_engine_config() -> void:
	# Disconnect signals 
	popups.save_config.disconnect("file_selected", self, "save_engine_config")
	popups.save_config.disconnect("popup_hide", self, "close_save_engine_config")

	# Hide save dialog
	popups.save_config.hide()

	opened_dialogs -= 1
	if opened_dialogs == 0:
		popups_root.hide()

# Called to close load engine config
func close_load_engine_config() -> void:
	# Disconnect signals 
	popups.load_config.disconnect("file_selected", self, "load_engine_config")
	popups.load_config.disconnect("popup_hide", self, "close_load_engine_config")

	# Hide load dialog
	popups.load_config.hide()

	opened_dialogs -= 1
	if opened_dialogs == 0:
		popups_root.hide()

# Called to close message dialog
func close_message_dialog() -> void:
	# Disconnect signals 
	popups.message_dialog.disconnect("popup_hide", self, "close_message_dialog")

	# Hide message dialog
	popups.message_dialog.hide()

	opened_dialogs -= 1
	if opened_dialogs == 0:
		popups_root.hide()
