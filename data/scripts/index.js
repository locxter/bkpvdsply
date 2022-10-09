'use strict';

// DOM references
let internalWheelDiameterInput = document.getElementById('internal-wheel-diameter');
let hubDiameterInput = document.getElementById('hub-diameter');
let imageInput = document.getElementById('image');
let convertButton = document.getElementById('convert');
let originalCanvas = document.getElementById('original');
let originalContext = originalCanvas.getContext('2d');
let bkpvdsplyCanvas = document.getElementById('bkpvdsply');
let bkpvdsplyContext = bkpvdsplyCanvas.getContext('2d');
let uploadButton = document.getElementById('upload');
let progressLabel = document.getElementById('progress');

// LED related constants
const LED_STRIP_LENGTH = 225;
const LED_COUNT = 32;
const LED_SPACING = LED_STRIP_LENGTH / LED_COUNT;

// Other variables
let internalWheelDiameter = 600;
let hubDiameter = 80;
let margin = ((internalWheelDiameter / 2) - (hubDiameter / 2) - LED_STRIP_LENGTH) / 2;
let originalImage = new Image();
let bkpvdsplyImage = new Array(360 * LED_COUNT);
for (let i = 0; i < bkpvdsplyImage.length; i++) {
    bkpvdsplyImage[i] = new Array(3);
}

// Function to upload the image angle by angle
async function uploadImage(image) {
    for (let i = 0; i < 360; i++) {
        let data = new URLSearchParams();
        let response;
        let responseText;
        data.append('angle', i);
        for (let j = 0; j < LED_COUNT; j++) {
            data.append('led-' + j + '-r', image[(i * LED_COUNT) + j][0]);
            data.append('led-' + j + '-g', image[(i * LED_COUNT) + j][1]);
            data.append('led-' + j + '-b', image[(i * LED_COUNT) + j][2]);
        }
        response = await fetch('http://192.168.0.1', {
            method: 'POST',
            body: data
        });
        responseText = await response.text();
        if (response.ok) {
            if (i < 359) {
                progressLabel.innerText = 'Progress: ' + Math.round((i / 360) * 100) + '%';
            } else {
                progressLabel.innerText = '';
            }
        } else {
            throw new Error('\nStatus: ' + response.status + '\nMessage: ' + responseText);
        }
    }
    return true;
}

// Handle convert button
convertButton.addEventListener('click', () => {
    if (imageInput.files[0]) {
        internalWheelDiameter = parseInt(internalWheelDiameterInput.value);
        hubDiameter = parseInt(hubDiameterInput.value);
        margin = ((internalWheelDiameter / 2) - (hubDiameter / 2) - LED_STRIP_LENGTH) / 2;
        originalImage.src = URL.createObjectURL(imageInput.files[0]);
    } else {
        alert('Please select an image before converting it.');
    }
});
// Handle image load
originalImage.addEventListener('load', () => {
    // Update canvas size
    originalCanvas.width = internalWheelDiameter;
    originalCanvas.height = internalWheelDiameter;
    bkpvdsplyCanvas.width = internalWheelDiameter;
    bkpvdsplyCanvas.height = internalWheelDiameter;
    // Draw original image
    originalContext.drawImage(originalImage, 0, 0, internalWheelDiameter, internalWheelDiameter);
    // Draw bkpvdsply background
    bkpvdsplyContext.fillStyle = '#000000';
    bkpvdsplyContext.fillRect(0, 0, internalWheelDiameter, internalWheelDiameter);
    // Draw bkpvdsply image and store the data for upload
    for (let i = 0; i < 360; i++) {
        for (let j = 0; j < LED_COUNT; j++) {
            let x = Math.round((internalWheelDiameter / 2) + (((hubDiameter / 2) + margin + (LED_SPACING / 2) + (j * LED_SPACING)) * Math.cos((i - 90) * (Math.PI / 180.0))));
            let y = Math.round((internalWheelDiameter / 2) + (((hubDiameter / 2) + margin + (LED_SPACING / 2) + (j * LED_SPACING)) * Math.sin((i - 90) * (Math.PI / 180.0))));
            let pixel = originalContext.getImageData(x, y, 1, 1);
            pixel.data[3] = 255;
            bkpvdsplyContext.putImageData(pixel, x, y);
            bkpvdsplyImage[(i * LED_COUNT) + j][0] = pixel.data[0];
            bkpvdsplyImage[(i * LED_COUNT) + j][1] = pixel.data[1];
            bkpvdsplyImage[(i * LED_COUNT) + j][2] = pixel.data[2];
        }
    }
    URL.revokeObjectURL(this.src);
});
// Handle upload button
uploadButton.addEventListener('click', () => {
    if (bkpvdsplyImage[0] !== undefined) {
        alert('Please be patient, the upload will take about 2 minutes.');
        uploadImage(bkpvdsplyImage)
            .then(() => {
                alert('Image successfully uploaded.');
            })
            .catch(error => {
                alert(error);
            });
    } else {
        alert('Please convert an image before uploading it.');
    }
});
