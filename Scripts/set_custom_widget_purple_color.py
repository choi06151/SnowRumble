import unreal


ASSET_PATH = "/Game/WBP/WBP_CustomWidget"
BUTTON_NAME = "PurpleBrushColorButton"
TARGET_COLOR = unreal.LinearColor(0.6509803921568628, 0.30980392156862746, 1.0, 1.0)


def _tinted_brush(brush, color):
    brush.tint_color = unreal.SlateColor(specified_color=color)
    return brush


def _set_button_style_color(button, color):
    style = button.get_editor_property("widget_style")
    style.normal = _tinted_brush(style.normal, color)
    style.hovered = _tinted_brush(style.hovered, color)
    style.pressed = _tinted_brush(style.pressed, color)
    style.disabled = _tinted_brush(style.disabled, color)
    button.set_editor_property("widget_style", style)


asset = unreal.EditorAssetLibrary.load_asset(ASSET_PATH)
if asset is None:
    raise RuntimeError(f"Asset not found: {ASSET_PATH}")

asset_name = ASSET_PATH.rsplit("/", 1)[-1]
candidate_paths = [
    f"{ASSET_PATH}.{asset_name}:WidgetTree.{BUTTON_NAME}",
    f"{ASSET_PATH}.{asset_name}_C:WidgetTree.{BUTTON_NAME}",
    f"{ASSET_PATH}.{asset_name}_C_CDO.WidgetTree.{BUTTON_NAME}",
]
button = None
for candidate_path in candidate_paths:
    button = unreal.load_object(None, candidate_path)
    if button is not None:
        unreal.log(f"Loaded button from {candidate_path}")
        break
if button is None:
    raise RuntimeError(f"Widget not found: {BUTTON_NAME}. Tried: {candidate_paths}")

_set_button_style_color(button, TARGET_COLOR)
unreal.EditorAssetLibrary.save_loaded_asset(asset)
unreal.log(f"Updated {ASSET_PATH}.{BUTTON_NAME} to #A64FFF")
