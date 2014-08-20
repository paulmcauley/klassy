//  Color manipulation utilities

function blendColors(clr0, clr1, p) {
    return Qt.tint(clr0, adjustAlpha(clr1, p));
}

function adjustAlpha(clr, a) {
    return Qt.rgba(clr.r, clr.g, clr.b, a);
}
