Follow the blender build instuctions. Use the following to build the external:

./blender/build_files/build_environment/install_deps.sh --source $PWD/external --install /opt/blender-panos/external --with-all --skip-osl

The command above will give CMAKE options. Add this as well -DOPENCOLLADA_ROOT_DIR

Commit the patch is based upon: 78a8d3685bd3487eb0b5dd55793f94fe7235e377
