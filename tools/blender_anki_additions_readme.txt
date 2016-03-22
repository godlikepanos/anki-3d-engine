Follow the blender build instuctions. Use the following to build the external:

./blender/build_files/build_environment/install_deps.sh --source $PWD/external --install /opt/blender-panos/external --with-all --skip-osl

The command above will give CMAKE options. Add this as well -DOPENCOLLADA_ROOT_DIR

Commit the patch is based upon: 2683dd8b93d6c14bb8b3bfa03650cd619d9dbb30
