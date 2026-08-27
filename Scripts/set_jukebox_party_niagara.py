import unreal


BLUEPRINT_PATH = "/Game/Blueprints/BP_JukeBox"
NIAGARA_PATH = "/Game/PartyFX/Niagara/NS_Paper_Party_Gold_Silver"
COMPONENT_NAME = "JukeboxNiagaraComponent"


def main():
    blueprint = unreal.EditorAssetLibrary.load_asset(BLUEPRINT_PATH)
    generated_class = unreal.EditorAssetLibrary.load_blueprint_class(BLUEPRINT_PATH)
    default_object = unreal.get_default_object(generated_class)
    niagara_system = unreal.EditorAssetLibrary.load_asset(NIAGARA_PATH)
    if not blueprint:
        raise RuntimeError(f"Blueprint not found: {BLUEPRINT_PATH}")
    if not generated_class or not default_object:
        raise RuntimeError(f"Blueprint class not found: {BLUEPRINT_PATH}")
    if not niagara_system:
        raise RuntimeError(f"Niagara system not found: {NIAGARA_PATH}")

    target_component = default_object.get_editor_property("jukebox_niagara_component")
    if not target_component:
        raise RuntimeError(f"Component not found: {COMPONENT_NAME}")

    target_component.set_editor_property("asset", niagara_system)
    if not unreal.EditorAssetLibrary.save_asset(BLUEPRINT_PATH, only_if_is_dirty=False):
        raise RuntimeError(f"Failed to save blueprint: {BLUEPRINT_PATH}")

    unreal.log(
        f"Set {BLUEPRINT_PATH}.{COMPONENT_NAME} Niagara asset to {NIAGARA_PATH}"
    )


main()
