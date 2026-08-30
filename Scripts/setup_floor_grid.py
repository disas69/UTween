import unreal

def setup_floor_grid():
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    editor_asset_lib = unreal.EditorAssetLibrary

    def save(asset):
        editor_asset_lib.save_loaded_asset(asset)

    # 1. Create or load M_GameFeel_FloorGrid
    m_path = "/Game/GameFeel/Materials/M_GameFeel_FloorGrid"
    m_floor = editor_asset_lib.load_asset(m_path)
    if not m_floor or not isinstance(m_floor, unreal.Material):
        m_floor = asset_tools.create_asset("M_GameFeel_FloorGrid", "/Game/GameFeel/Materials", unreal.Material, unreal.MaterialFactoryNew())

    if m_floor:
        unreal.MaterialEditingLibrary.delete_all_material_expressions(m_floor)
        m_floor.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)
        m_floor.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)

        # TexCoord
        tex_coord = unreal.MaterialEditingLibrary.create_material_expression(m_floor, unreal.MaterialExpressionTextureCoordinate, -1200, 0)
        
        # GridTiling
        tiling = unreal.MaterialEditingLibrary.create_material_expression(m_floor, unreal.MaterialExpressionScalarParameter, -1200, 150)
        tiling.set_editor_property("parameter_name", "GridTiling")
        tiling.set_editor_property("default_value", 30.0)

        # Multiply TexCoord * GridTiling
        mult_tiling = unreal.MaterialEditingLibrary.create_material_expression(m_floor, unreal.MaterialExpressionMultiply, -1000, 50)
        unreal.MaterialEditingLibrary.connect_material_expressions(tex_coord, "", mult_tiling, "A")
        unreal.MaterialEditingLibrary.connect_material_expressions(tiling, "", mult_tiling, "B")

        # Floor(TexCoord * GridTiling)
        floor_node = unreal.MaterialEditingLibrary.create_material_expression(m_floor, unreal.MaterialExpressionFloor, -850, 50)
        unreal.MaterialEditingLibrary.connect_material_expressions(mult_tiling, "", floor_node, "")

        # Component Mask X (R)
        mask_x = unreal.MaterialEditingLibrary.create_material_expression(m_floor, unreal.MaterialExpressionComponentMask, -700, -50)
        mask_x.set_editor_property("r", True)
        mask_x.set_editor_property("g", False)
        mask_x.set_editor_property("b", False)
        mask_x.set_editor_property("a", False)
        unreal.MaterialEditingLibrary.connect_material_expressions(floor_node, "", mask_x, "")

        # Component Mask Y (G)
        mask_y = unreal.MaterialEditingLibrary.create_material_expression(m_floor, unreal.MaterialExpressionComponentMask, -700, 150)
        mask_y.set_editor_property("r", False)
        mask_y.set_editor_property("g", True)
        mask_y.set_editor_property("b", False)
        mask_y.set_editor_property("a", False)
        unreal.MaterialEditingLibrary.connect_material_expressions(floor_node, "", mask_y, "")

        # Add X + Y
        add_xy = unreal.MaterialEditingLibrary.create_material_expression(m_floor, unreal.MaterialExpressionAdd, -550, 50)
        unreal.MaterialEditingLibrary.connect_material_expressions(mask_x, "", add_xy, "A")
        unreal.MaterialEditingLibrary.connect_material_expressions(mask_y, "", add_xy, "B")

        # Multiply by 0.5
        const_half = unreal.MaterialEditingLibrary.create_material_expression(m_floor, unreal.MaterialExpressionConstant, -550, 200)
        const_half.set_editor_property("r", 0.5)

        mult_half = unreal.MaterialEditingLibrary.create_material_expression(m_floor, unreal.MaterialExpressionMultiply, -400, 100)
        unreal.MaterialEditingLibrary.connect_material_expressions(add_xy, "", mult_half, "A")
        unreal.MaterialEditingLibrary.connect_material_expressions(const_half, "", mult_half, "B")

        # Frac
        frac_node = unreal.MaterialEditingLibrary.create_material_expression(m_floor, unreal.MaterialExpressionFrac, -250, 100)
        unreal.MaterialEditingLibrary.connect_material_expressions(mult_half, "", frac_node, "")

        # Multiply Frac * 2.0 (yields 0.0 or 1.0)
        const_two = unreal.MaterialEditingLibrary.create_material_expression(m_floor, unreal.MaterialExpressionConstant, -250, 250)
        const_two.set_editor_property("r", 2.0)

        checker_alpha = unreal.MaterialEditingLibrary.create_material_expression(m_floor, unreal.MaterialExpressionMultiply, -100, 150)
        unreal.MaterialEditingLibrary.connect_material_expressions(frac_node, "", checker_alpha, "A")
        unreal.MaterialEditingLibrary.connect_material_expressions(const_two, "", checker_alpha, "B")

        # GridColorA (Light Gray)
        col_a = unreal.MaterialEditingLibrary.create_material_expression(m_floor, unreal.MaterialExpressionVectorParameter, -300, -200)
        col_a.set_editor_property("parameter_name", "GridColorA")
        col_a.set_editor_property("default_value", unreal.LinearColor(0.42, 0.42, 0.42, 1.0))

        # GridColorB (Darker Gray)
        col_b = unreal.MaterialEditingLibrary.create_material_expression(m_floor, unreal.MaterialExpressionVectorParameter, -300, -50)
        col_b.set_editor_property("parameter_name", "GridColorB")
        col_b.set_editor_property("default_value", unreal.LinearColor(0.24, 0.24, 0.24, 1.0))

        # Lerp(GridColorA, GridColorB, checker_alpha)
        lerp_node = unreal.MaterialEditingLibrary.create_material_expression(m_floor, unreal.MaterialExpressionLinearInterpolate, 100, -50)
        unreal.MaterialEditingLibrary.connect_material_expressions(col_a, "", lerp_node, "A")
        unreal.MaterialEditingLibrary.connect_material_expressions(col_b, "", lerp_node, "B")
        unreal.MaterialEditingLibrary.connect_material_expressions(checker_alpha, "", lerp_node, "Alpha")

        # Connect to BaseColor
        unreal.MaterialEditingLibrary.connect_material_property(lerp_node, "", unreal.MaterialProperty.MP_BASE_COLOR)

        # Roughness
        roughness = unreal.MaterialEditingLibrary.create_material_expression(m_floor, unreal.MaterialExpressionScalarParameter, 0, 200)
        roughness.set_editor_property("parameter_name", "Roughness")
        roughness.set_editor_property("default_value", 0.5)
        unreal.MaterialEditingLibrary.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)

        unreal.MaterialEditingLibrary.recompile_material(m_floor)
        save(m_floor)
        print("M_GameFeel_FloorGrid created and compiled successfully!")

    # 2. Update MI_GameFeel_FloorGrid
    mi_path = "/Game/GameFeel/Materials/MI_GameFeel_FloorGrid"
    mi_floor = editor_asset_lib.load_asset(mi_path)
    if not mi_floor or not isinstance(mi_floor, unreal.MaterialInstanceConstant):
        mi_floor = asset_tools.create_asset("MI_GameFeel_FloorGrid", "/Game/GameFeel/Materials", unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())

    if mi_floor:
        mi_floor.set_editor_property("parent", m_floor)
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(mi_floor, "GridColorA", unreal.LinearColor(0.42, 0.42, 0.42, 1.0))
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(mi_floor, "GridColorB", unreal.LinearColor(0.24, 0.24, 0.24, 1.0))
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(mi_floor, "GridTiling", 30.0)
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(mi_floor, "Roughness", 0.5)
        unreal.MaterialEditingLibrary.update_material_instance(mi_floor)
        save(mi_floor)
        print("MI_GameFeel_FloorGrid updated successfully!")

    print("Checkers floor grid setup complete!")

setup_floor_grid()