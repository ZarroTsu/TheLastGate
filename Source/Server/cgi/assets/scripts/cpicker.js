const pickr = Pickr.create({
    el: '.color-picker',
    theme: 'nano',
    useAsButton: true,
    lockOpacity: true,
    comparison: false,
    default: '#000000',
    components: {
        // Main components
        preview: true,
        hue: true,
        // Input / output Options
        interaction: {
            hex: true,
            rgba: true,
            input: true
        }
    }
});

const pickr_button = document.getElementById('button-temp-pickr');
pickr.on('change', (color, instance) => {
    var col = color.toRGBA();
    pickr_button.style.backgroundColor = `rgb(${col[0]}, ${col[1]}, ${col[2]})`;
});