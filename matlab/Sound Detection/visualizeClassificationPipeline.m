function visualizeClassificationPipeline(audioData,net,labels)

% Unpack audio data
audio = audioData{1};
fs = audioData{2};
knownlabel = audioData{3};

% Create tiled layout
tiledlayout(3,1)

% Plot audio signal in first tile
nexttile
plotAudio(audio,fs)
title("Known Class = "+knownlabel)

% Plot auditory spectrogram in second tile
nexttile
auditorySpectrogram = extractAuditorySpectrogram(audio,fs);
plotAuditorySpectrogram(auditorySpectrogram)

% Plot network predictions as word cloud in third tile
nexttile
scores = predict(net,auditorySpectrogram);
prediction = scores2label(scores,labels,2);
wordcloud(labels,scores)
title("Predicted Class = "+string(prediction))

    function plotAuditorySpectrogram(auditorySpectrogram)
        %plotAuditorySpectrogram Plot auditory spectrogram

        % extractAuditorySpectrogram uses 25 ms windows with 10 ms hops.
        % Create a time vector with instants corresponding to the center of
        % the windows
        t = 0.0125:0.01:(1-0.0125);

        bins = 1:size(auditorySpectrogram,2);

        pcolor(t,bins,auditorySpectrogram')
        shading flat
        xlabel("Time (s)")
        ylabel("Bark (bins)")

    end
    function plotAudio(audioIn,fs)
        %plotAudio Plot audio

        t = (0:size(audioIn,1)-1)/fs;
        plot(t,audioIn)
        xlabel("Time (s)")
        ylabel("Amplitude")
        grid on
        axis tight

    end
end