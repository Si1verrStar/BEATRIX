function features = extractAuditorySpectrogram(x,fs)

% Design audioFeatureExtractor object
persistent afe segmentSamples
if isempty(afe)
    designFs = 16e3;
    segmentDuration = 1;
    frameDuration = 0.025;
    hopDuration = 0.01;

    numBands = 50;
    FFTLength = 512;

    segmentSamples = round(segmentDuration*designFs);
    frameSamples = round(frameDuration*designFs);
    hopSamples = round(hopDuration*designFs);
    overlapSamples = frameSamples - hopSamples;

    afe = audioFeatureExtractor( ...
        SampleRate=designFs, ...
        FFTLength=FFTLength, ...
        Window=hann(frameSamples,"periodic"), ...
        OverlapLength=overlapSamples, ...
        barkSpectrum=true);
    setExtractorParameters(afe,"barkSpectrum",NumBands=numBands,WindowNormalization=false);
end

% Resample to 16 kHz if necessary
if double(fs)~=16e3
    x = cast(resample(double(x),16e3,double(fs)),like=x);
end

% Ensure the input is equal to 1 second of data at 16 kHz.
x = resize(x,segmentSamples,Side="both");

% Extract features
features = extract(afe,x);

% Apply logarithm
features = log10(features + 1e-6);

end