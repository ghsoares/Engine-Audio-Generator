extends Control

onready var id_label: Label = $ID
onready var cavity_length_slider: TextSlider = $VBox/CavityLength

var id: int = -1 setget set_id
onready var delete_button: Button = $Delete
var cavity_length: float = -1.0 setget set_cavity_length

signal changed(element)
signal deleted(element)

func _ready() -> void:
	delete_button.connect("pressed", self, "on_delete_pressed")
	cavity_length_slider.connect("value_changed", self, "set_cavity_length")

func set_id(val: int) -> void:
	if val != id:
		id = val
		id_label.text = "%d." % (id + 1)

func on_delete_pressed() -> void:
	emit_signal("deleted", self)

func set_cavity_length(val: float) -> void:
	if val != cavity_length:
		cavity_length = val

		cavity_length_slider.set_block_signals(true)

		cavity_length_slider.value = cavity_length
		cavity_length_slider.text = "Cavity Length: %.2f Meters" % cavity_length
		emit_signal("changed", self)

		cavity_length_slider.set_block_signals(false)



