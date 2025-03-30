# blender_glass_cup.py
import bpy


bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)

bpy.ops.mesh.primitive_cylinder_add(
    vertices=64, 
    radius=1.0, 
    depth=2.0, 
    location=(0, 0, 1.0)
)
outer = bpy.context.active_object
outer.name = "Cup_Outer"

bpy.ops.mesh.primitive_cylinder_add(
    vertices=64, 
    radius=0.85, 
    depth=1.8, 
    location=(0, 0, 1.0)
)
inner = bpy.context.active_object
inner.name = "Cup_Inner"

bpy.context.view_layer.objects.active = outer
bool_mod = outer.modifiers.new(name="Cup_Boolean", type='BOOLEAN')
bool_mod.operation = 'DIFFERENCE'
bool_mod.object = inner
bpy.ops.object.modifier_apply(modifier=bool_mod.name)
bpy.data.objects.remove(inner, do_unlink=True)

bpy.ops.mesh.primitive_torus_add(
    major_radius=0.6,
    minor_radius=0.1,
    major_segments=48,
    minor_segments=12,
    location=(1.0, 0, 1.0),
    rotation=(0, 0, 1.57)
)
handle = bpy.context.active_object
handle.name = "Cup_Handle"

bpy.ops.object.select_all(action='DESELECT')
outer.select_set(True)
handle.select_set(True)
bpy.context.view_layer.objects.active = outer
bpy.ops.object.join()

mat = bpy.data.materials.new(name="GlassMaterial")
mat.use_nodes = True
nodes = mat.node_tree.nodes
links = mat.node_tree.links
for node in nodes:
    nodes.remove(node)
output_node = nodes.new(type='ShaderNodeOutputMaterial')
bsdf_node = nodes.new(type='ShaderNodeBsdfPrincipled')
bsdf_node.inputs["Transmission"].default_value = 1.0
bsdf_node.inputs["Roughness"].default_value = 0.0
bsdf_node.inputs["IOR"].default_value = 1.45
bsdf_node.inputs["Base Color"].default_value = (1, 1, 1, 1)
links.new(bsdf_node.outputs["BSDF"], output_node.inputs["Surface"])
outer.data.materials.append(mat)

# Smooth shading and set origin at bottom
for poly in outer.data.polygons:
    poly.use_smooth = True
bpy.ops.object.origin_set(type='ORIGIN_GEOMETRY', center='BOUNDS')
outer.location.z -= outer.dimensions.z / 2.0

print("Glass cup created successfully.")
