extends Control
class_name Playback

var acc: float
var rpm: float

export var idle_rpm: float = 1000.0
export var max_rpm: float = 8000.0
export var rev_rpm: float = 200.0
export var acceleration: float = 16000.0
export var decceleration: float = 8000.0
export var combustion_volume: float = 2.0

export (Array, AudioEffect) var effects: Array
onready var engine: AudioStreamPlayer = $Engine

func _ready() -> void:
	print(AudioServer.get_time_to_next_mix())

	rpm = idle_rpm
	update_engine(1.0)

	for eff in effects:
		engine.add_effect(eff)
	
	engine.play()

func update_engine(delta: float) -> void:
	engine.process_audio(delta)

func _process(delta: float) -> void:
	var acc: float = Input.get_action_strength("accelerate")

	var acc_rpm: float = (acceleration + decceleration) * delta
	var dec_rpm: float = (decceleration) * delta

	var dif: float
	dif = idle_rpm - rpm
	dif = clamp(dif, -dec_rpm, dec_rpm)
	rpm += dif

	dif = (max_rpm + rev_rpm) - rpm
	dif = clamp(dif, 0.0, acc_rpm * acc)
	rpm += dif
	
	engine.max_rpm = max_rpm
	engine.rpm = rpm
	engine.rev_inset = (rev_rpm * 2.0) / acceleration
	engine.rev_outset = (rev_rpm * 2.0) / decceleration

	engine.combustion_volume = combustion_volume * acc

	update_engine(delta)
