'use strict';

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

let internalWheelDiameter = 600;
let hubDiameter = 80;
let ledStripLength = 250;
let ledCount = 36;
let ledSpacing = ledStripLength / ledCount;
let margin = ((internalWheelDiameter / 2) - (hubDiameter / 2) - ledStripLength) / 2;
let originalImage = new Image();
let bkpvdsplyImage = new Array(12960);
for (let i = 0; i < bkpvdsplyImage.length; i++) {
    bkpvdsplyImage[i] = new Array(3);
}

async function uploadImage(image) {
    for (let i = 0; i < 360; i++) {
        let data = new URLSearchParams();
        let response;
        let responseText;
        data.append('angle', i);
        for (let j = 0; j < 36; j++) {
            data.append('led-' + j + '-r', image[(i * 36) + j][0]);
            data.append('led-' + j + '-g', image[(i * 36) + j][1]);
            data.append('led-' + j + '-b', image[(i * 36) + j][2]);
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

convertButton.addEventListener('click', () => {
    if (imageInput.files[0]) {
        internalWheelDiameter = parseInt(internalWheelDiameterInput.value);
        hubDiameter = parseInt(hubDiameterInput.value);
        margin = ((internalWheelDiameter / 2) - (hubDiameter / 2) - ledStripLength) / 2;
        originalImage.src = URL.createObjectURL(imageInput.files[0]);
    } else {
        alert('Please select an image before converting it.');
    }
});

originalImage.addEventListener('load', () => {
    originalCanvas.width = internalWheelDiameter;
    originalCanvas.height = internalWheelDiameter;
    bkpvdsplyCanvas.width = internalWheelDiameter;
    bkpvdsplyCanvas.height = internalWheelDiameter;

    originalContext.drawImage(originalImage, 0, 0, internalWheelDiameter, internalWheelDiameter);

    bkpvdsplyContext.fillStyle = '#000000';
    bkpvdsplyContext.fillRect(0, 0, internalWheelDiameter, internalWheelDiameter);

    for (let i = 0; i < 360; i++) {
        for (let j = 0; j < ledCount; j++) {
            let x = Math.round((internalWheelDiameter / 2) + (((hubDiameter / 2) + margin + (ledSpacing / 2) + (j * ledSpacing)) * Math.cos((i - 90) * (Math.PI / 180.0))));
            let y = Math.round((internalWheelDiameter / 2) + (((hubDiameter / 2) + margin + (ledSpacing / 2) + (j * ledSpacing)) * Math.sin((i - 90) * (Math.PI / 180.0))));
            let pixel = originalContext.getImageData(x, y, 1, 1);
            pixel.data[3] = 255;
            bkpvdsplyContext.putImageData(pixel, x, y);
            bkpvdsplyImage[(i * 36) + j][0] = pixel.data[0];
            bkpvdsplyImage[(i * 36) + j][1] = pixel.data[1];
            bkpvdsplyImage[(i * 36) + j][2] = pixel.data[2];
        }
    }
    URL.revokeObjectURL(this.src);
});

uploadButton.addEventListener('click', () => {
    if (bkpvdsplyImage[0] !== undefined) {
        alert('Please be patient, the upload will take about 4 minutes.');
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
