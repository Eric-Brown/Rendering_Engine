# Get all the shader files in the current directory.
$Shader_Files = Get-ChildItem -File *.frag, *.vert
# Iterator through each of them, calling glslangValidator
# Output the files into the "sprvs" directory
foreach ($info_obj in $Shader_Files)
{
    $expression = "glslangValidator -V {0} -o sprvs/{1}.spv" -f $info_obj.Name,$info_obj.Name.Replace(".", "_")
    echo $expression
    Invoke-Expression $expression
}
