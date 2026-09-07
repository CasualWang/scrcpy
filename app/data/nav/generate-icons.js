const fs = require('fs');
const path = require('path');
const sharp = require('sharp');
const {
    Home,
    Back,
    AppSwitch,
    Power,
    VolumeUp,
    VolumeDown,
    Moon,
    SunOne,
    ScreenRotation,
    ReverseRotation,
} = require('@icon-park/svg');

const ICONS = [
    ['nav_home.png', Home],
    ['nav_back.png', Back],
    ['nav_recents.png', AppSwitch],
    ['nav_power.png', Power],
    ['nav_screen_off.png', Moon],
    ['nav_screen_on.png', SunOne],
    ['nav_volume_up.png', VolumeUp],
    ['nav_volume_down.png', VolumeDown],
    ['nav_rotate_device.png', ScreenRotation],
    ['nav_rotate_left.png', ReverseRotation],
];

const OUT_DIR = __dirname;
const OPTIONS = {
    theme: 'outline',
    size: 24,
    fill: '#FFFFFF',
    strokeWidth: 3,
};

async function writeIcon(filename, IconFn, { flop = false } = {}) {
    const svg = IconFn(OPTIONS);
    const outPath = path.join(OUT_DIR, filename);
    let pipeline = sharp(Buffer.from(svg));
    if (flop) {
        pipeline = pipeline.flop();
    }
    await pipeline.png().toFile(outPath);
    console.log('Wrote', outPath);
}

async function main() {
    for (const [filename, IconFn] of ICONS) {
        await writeIcon(filename, IconFn);
    }
    await writeIcon('nav_rotate_right.png', ReverseRotation, { flop: true });
}

main().catch((err) => {
    console.error(err);
    process.exit(1);
});
