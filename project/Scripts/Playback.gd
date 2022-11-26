extends Control
class_name Playback

var rpm: float
var speed: float
var acc: float
var gear: int
var gear_change: int
var gear_switch_timer: float
var revving: bool

onready var gear_label: Label = $"UI/VBox/Gear"
onready var gas_progress: ProgressBar = $"UI/VBox/Gas"
onready var rpm_progress: ProgressBar = $"UI/VBox/RPM"
onready var rpm_label: Label = $"UI/VBox/RPM/Label"
onready var speed_label: Label = $"UI/VBox/Speed"

onready var engine_player: AudioStreamPlayer = $Engine
var engine_audio_player

export var min_rpm: float = 1000.0
export var max_rpm: float = 8000.0
export var margin_rpm: float = 200.0

export var gear_count: int = 6
export var gear_switch_delay: float = 0.1

export var speed_range: Vector2 = Vector2(30.0, 120.0)
export var acceleration_range: Vector2 = Vector2(4000, 1000)
export var decceleration_range: Vector2 = Vector2(2000, 500)
export (float, EASE) var speed_curve: float = 1.0
export (float, EASE) var acceleration_curve: float = 1.0
export (float, EASE) var decceleration_curve: float = 1.0
export var speed_decceleration: float = 8.0

export var master_volume: float = 1.0
export var crankshaft_stream: AudioStreamSample
export var ignition_stream: AudioStreamSample
export var exhaust_stream: AudioStreamSample
export var crankshaft_volume: float = 1.0
export var ignition_volume_range: Vector2 = Vector2(0.1, 1.0)
export var exhaust_volume_range: Vector2 = Vector2(0.1, 1.0)
export var revving_master_volume: float = 0.1
export var rpm_blend: float = 16000.0
export var volume_blend: float = 128.0

export (Array, AudioEffect) var effects: Array

func _ready() -> void:
	rpm = min_rpm
	
	engine_audio_player = EngineAudioPlayer.new()
	engine_audio_player.audio_generator = engine_player.stream
	engine_audio_player.audio_generator_playback = engine_player.get_stream_playback()
	engine_audio_player.crankshaft_stream = crankshaft_stream
	engine_audio_player.ignition_stream = ignition_stream
	engine_audio_player.exhaust_stream = exhaust_stream
	engine_audio_player.rpm_blend = rpm_blend
	engine_audio_player.volume_blend = volume_blend
	engine_audio_player.rpm = rpm

	update_engine(1.0)

	engine_player.play()

	gear = 1

func update_engine(delta: float) -> void:
	engine_audio_player.process_audio(delta)

func _process(delta: float) -> void:
	acc = Input.get_action_strength("accelerate")
	if Input.is_action_just_pressed("gear_up"):
		gear_change += 1
	if Input.is_action_just_pressed("gear_down"):
		gear_change -= 1

func _physics_process(delta: float) -> void:
	var gear_t: float = (gear - 1.0) / (gear_count - 1.0)
	var acceleration: float = lerp(acceleration_range.x, acceleration_range.y, ease(gear_t, acceleration_curve))
	var decceleration: float = lerp(decceleration_range.x, decceleration_range.y, ease(gear_t, decceleration_curve))
	var speed: float = lerp(speed_range.x, speed_range.y, ease(gear_t, speed_curve))

	if gear_switch_timer > 0:
		self.speed += clamp(-self.speed, -speed_decceleration * delta, speed_decceleration * delta)
		
	var spd_t: float = self.speed / speed
	rpm = min_rpm + (max_rpm - min_rpm) * spd_t

	gear_change = clamp(gear + gear_change, 1, gear_count) - gear

	if gear_change != 0:
		gear += gear_change
		gear_switch_timer = gear_switch_delay
		gear_change = 0

	if not revving:
		if rpm >= max_rpm:
			revving = true
	if revving:
		var m_rpm: float = margin_rpm * (acceleration / acceleration_range.x)
		if rpm <= max_rpm - m_rpm:
			revving = false

	if gear_switch_timer > 0:
		acc = 0

	var acc_rpm: float = (acceleration + decceleration) * delta * acc
	var dec_rpm: float = (decceleration) * delta

	if revving:
		acc_rpm = 0

	rpm += clamp(min_rpm - rpm, -dec_rpm, dec_rpm)
	rpm += clamp(max_rpm - rpm, 0.0, acc_rpm)

	if gear_switch_timer > 0:
		gear_switch_timer -= delta
	else:
		var rpm_t: float = (rpm - min_rpm) / (max_rpm - min_rpm)
		self.speed = rpm_t * speed

	engine_audio_player.rpm = rpm
	engine_audio_player.master_volume = master_volume
	engine_audio_player.crankshaft_volume = crankshaft_volume
	engine_audio_player.ignition_volume = lerp(ignition_volume_range.x, ignition_volume_range.y, acc)
	engine_audio_player.exhaust_volume = lerp(exhaust_volume_range.x, exhaust_volume_range.y, acc)

	if revving:
		engine_audio_player.master_volume *= revving_master_volume

	update_engine(delta)
	update_ui(delta)

func update_ui(delta: float) -> void:
	var spd: float = 3.6 * speed
	spd /= 1.609

	gear_label.text = "%d" % gear

	gas_progress.min_value = 0
	gas_progress.max_value = 1
	gas_progress.value = lerp(gas_progress.value, acc, 32 * delta)

	rpm_progress.min_value = min_rpm
	rpm_progress.max_value = max_rpm
	rpm_progress.value = lerp(rpm_progress.value, rpm, 32 * delta)

	rpm_label.text = "%d RPM" % floor(rpm)
	speed_label.text = "%d MPH" % floor(spd)
