import unreal


blueprint = unreal.EditorAssetLibrary.load_asset("/Game/Blueprints/BP_JukeBox")
generated_class = unreal.EditorAssetLibrary.load_blueprint_class(
    "/Game/Blueprints/BP_JukeBox"
)
default_object = unreal.get_default_object(generated_class)

blueprint_names = [
    name
    for name in dir(blueprint)
    if "component" in name.lower()
    or "construction" in name.lower()
    or "script" in name.lower()
    or "subobject" in name.lower()
    or "editor" in name.lower()
]
default_names = [
    name
    for name in dir(default_object)
    if "component" in name.lower()
    or "niagara" in name.lower()
    or "asset" in name.lower()
    or "editor" in name.lower()
]
unreal_names = [
    name
    for name in dir(unreal)
    if "Subobject" in name
    or "Blueprint" in name
    or "Component" in name
    or "Kismet" in name
]

raise RuntimeError(
    "blueprint methods: "
    + ", ".join(sorted(blueprint_names))
    + "\ndefault methods: "
    + ", ".join(sorted(default_names))
    + "\nunreal helpers: "
    + ", ".join(sorted(unreal_names))[:6000]
)
