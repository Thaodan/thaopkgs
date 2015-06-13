#!/bin/bash

if [ $# -lt 1 -o \( "$1" = "-h" -o "$1" = "--help" \) ]; then
	echo "usage: $0 <path/to/package>"
	echo "(should be done in root of the original repository)"
	exit -1
fi

path=${1%/}		# the trailing / is removed
pkg=`basename $path`
email=`git config user.email`

# remove the path from version control
echo git rm -r \"$path\"
git rm -r --cached "$path"

# backup uncommited files like previously built packages (OPTIONAL)
echo mv \"$path\" \"${pkg}_aur4_bak\"
mv "$path" "${pkg}_aur4_bak"

# add the AUR 4 package as a submodule at the previous path
echo git submodule add ssh+git://aur@aur4.archlinux.org/${pkg}.git $path
git submodule add -f ssh+git://aur@aur4.archlinux.org/${pkg}.git $path


# add a separate pushurl for write access
# (OPTIONAL, you can also just use the url above)

# (we need to be in the root of the repository to find .gitmodules)
echo git config --file=.gitmodules submodule.${path}.pushurl \
	ssh+git://aur@aur4.archlinux.org/${pkg}.git $path
git config --file=.gitmodules submodule.${path}.pushurl \
	ssh+git://aur@aur4.archlinux.org/${pkg}.git $path

# write urls from .gitmodules to separate .git/modules/*/config files
echo git submodule sync
git submodule sync

# mark changes in .gitmodules to be committed
echo git add .gitmodules
git add .gitmodules


# restore uncommited/unversioned files from backup (OPTIONAL)
echo cp -r \"${pkg}_aur4_bak/\"* \"$path\"
#mkdir $path
cp -r "${pkg}_aur4_bak/"* "$path"

# remove backup directory (OPTIONAL)
echo rm -rf \"${pkg}_aur4_bak/\"
rm -rf "${pkg}_aur4_bak"


# change to new submodule
echo pushd ${path}
pushd ${path}

  # set same commit email as for the original repository (OPTIONAL)
  echo git config user.email $email
  git config user.email $email
  mksrcinfo
  git add *
  git add .SRCINFO
  git commit -m "initial import"
  git push origin master
# return to original directory
echo popd
popd
