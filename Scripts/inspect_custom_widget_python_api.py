import unreal


asset = unreal.EditorAssetLibrary.load_asset("/Game/WBP/WBP_CustomWidget")
unreal.log(f"asset type: {asset.get_class().get_name()}")
generated_class = unreal.EditorAssetLibrary.load_blueprint_class("/Game/WBP/WBP_CustomWidget")
default_object = unreal.get_default_object(generated_class)

method_names = [name for name in dir(asset) if "class" in name.lower() or "widget" in name.lower() or "tree" in name.lower()]
interesting = "interesting methods: " + ", ".join(sorted(method_names))
default_methods = [
    name for name in dir(default_object)
    if "widget" in name.lower() or "name" in name.lower() or "tree" in name.lower()
]

unreal_names = [
    name for name in dir(unreal)
    if "WidgetBlueprint" in name or "WidgetTree" in name or "UserWidget" in name
]
helpers = "unreal helpers: " + ", ".join(sorted(unreal_names))

raise RuntimeError(interesting + "\ndefault methods: " + ", ".join(sorted(default_methods)) + "\n" + helpers[:3500])
