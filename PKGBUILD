# Maintainer: Jérôme Poulin <jeromepoulin@gmail.com>
pkgname=corosync-quorum-notify
pkgver=1.1
pkgrel=1
pkgdesc="Notify external program upon quorum state change in a Corosync cluster"
arch=('x86_64')
url='https://github.com/ticpu/corosync-quorum-notify'
license=('BSD')
makedepends=('cmake')
depends=('systemd' 'corosync>=3.1.0')
source=("corosync-quorum-notify::git+https://github.com/ticpu/corosync-quorum-notify.git")
sha256sums=('SKIP')

build() {
  cd "$srcdir"

  rm -rf build
  mkdir build
  cd build
  cmake ../$pkgname
  make
}

package() {
  cd "$srcdir/build"

  install -Dm755 corosync-quorum-notify "$pkgdir/usr/bin/corosync-quorum-notify"
}
