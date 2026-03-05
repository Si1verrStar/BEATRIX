function label = detectCommands(options) %WPFC20 modification to allow us to see the label

arguments
    options.SampleRate
    options.Network
    options.Labels
    options.ClassificationRate
    options.DecisionTimeWindow
    options.FrameAgreementThreshold
    options.ProbabilityThreshold
    options.Input = []
    options.TimeLimit = inf
    options.Allowed_CMDS
    options.display
end

label = "background";

% Isolate the labels
labels = options.Labels;

if isempty(options.Input)
    % Create an audioDeviceReader to read audio from your microphone.
    adr = audioDeviceReader(SampleRate=options.SampleRate,SamplesPerFrame=floor(options.SampleRate/options.ClassificationRate));

    % Create a dsp.AsyncBuffer to buffer the audio streaming from your
    % microphone into overlapping segments.
    audioBuffer = dsp.AsyncBuffer(options.SampleRate);
else
    % Create a dsp.AsyncBuffer object. Write the audio to the buffer so that
    % you can read from it in a streaming fashion.
    audioBuffer = dsp.AsyncBuffer(size(options.Input,1));
    write(audioBuffer,options.Input);

    % Create an audioDeviceWriter object to write the audio to your default
    % speakers in a streaming loop.
    adw = audioDeviceWriter(SampleRate=options.SampleRate);
end

newSamplesPerUpdate = floor(options.SampleRate/options.ClassificationRate);

% Convert the requested decision time window to the number of analysis frames.
numAnalysisFrame = round((options.DecisionTimeWindow-1)*(options.ClassificationRate) + 1);

% Convert the requested frame agreement threshold in percent to the number of frames that must agree.
countThreshold = round(options.FrameAgreementThreshold/100*numAnalysisFrame);

% Initialize buffers for the classification decisions and scores of the streaming audio.
YBuffer = repmat(categorical("background"),numAnalysisFrame,1);
scoreBuffer = zeros(numel(labels),numAnalysisFrame,"single");

% Create a timescope object to visualize the audio processed in the
% streaming loop. Create a dsp.MatrixViewer object to visualize the
% auditory spectrogram used to make predictions.
wavePlotter = timescope( ...
    SampleRate=options.SampleRate, ...
    Title="...", ...
    TimeSpanSource="property", ...
    TimeSpan=1, ...
    YLimits=[-1,1], ...
    Position=[600,640,800,340], ...
    TimeAxisLabels="none", ...
    AxesScaling="manual");

specPlotter = dsp.MatrixViewer( ...
    XDataMode="Custom", ...
    AxisOrigin="Lower left corner", ...
    Position=[600,220,800,380], ...
    ShowGrid=false, ...
    Title="...", ...
    XLabel="Time (s)", ...
    YLabel="Bark (bin)");
if options.display == true  %WFPC20 to stop showing the figure
    show(wavePlotter) 
    show(specPlotter)
end

% Initialize variables for plotting
currentTime = 0;
colorLimits = [-1,1];

% Run the streaming loop.
loopTimer = tic;
while whileCriteria(loopTimer,options.TimeLimit,wavePlotter,specPlotter,options.Input,audioBuffer)

    if isempty(options.Input)
        % Extract audio samples from the audio device and add the samples to
        % the buffer.
        audioIn = adr();
        write(audioBuffer,audioIn);
    end

    % Read samples from the buffer
    y = read(audioBuffer,options.SampleRate,options.SampleRate - newSamplesPerUpdate);
    
    % Extract an auditory spectrogram from the audio
    spec = extractAuditorySpectrogram(y,options.SampleRate);
    
    % Classify the current spectrogram, save the label to the label buffer,
    % and save the predicted probabilities to the probability buffer.
    score = predict(options.Network,spec);
    YPredicted = scores2label(score,labels,2);
    YBuffer = [YBuffer(2:end);YPredicted];
    scoreBuffer = [scoreBuffer(:,2:end),score(:)];
    
    % Plot the current waveform and spectrogram.
    ynew = y(end-newSamplesPerUpdate+1:end);
    wavePlotter(ynew)
    specPlotter(spec')

    % Declare a detection and display it in the figure if the following hold: 
    %   1) The most common label is not background. 
    %   2) At least countThreshold of the latest frame labels agree. 
    %   3) The maximum probability of the predicted label is at least probThreshold.
    % Otherwise, do not declare a detection.
    [YMode,count] = mode(YBuffer);
    maxProb = max(scoreBuffer(labels == YMode,:));
   if YMode == "background" || count < countThreshold || maxProb < options.ProbabilityThreshold
        wavePlotter.Title = "...";
        specPlotter.Title = "...";

    elseif ~ismember(string(YMode),options.Allowed_CMDS)
        wavePlotter.Title = "...";
        specPlotter.Title = "...";
    else
        label = string(YMode);
        wavePlotter.Title = label;
        specPlotter.Title = label; %WFPC20 exit the detection loop when a command is found
        break
    end
    
    % Update variables for plotting
    currentTime = currentTime + newSamplesPerUpdate/options.SampleRate;
    colorLimits = [min([colorLimits(1),min(spec,[],"all")]),max([colorLimits(2),max(spec,[],"all")])];
    specPlotter.CustomXData = [currentTime-1,currentTime];
    specPlotter.ColorLimits = colorLimits;

    if ~isempty(options.Input)
        % Write the new audio to your audio output device.
        adw(ynew);
    end
end
release(wavePlotter)
release(specPlotter)

    function tf = whileCriteria(loopTimer,timeLimit,wavePlotter,specPlotter,Input,audioBuffer)
        if isempty(Input)
            tf = toc(loopTimer)<timeLimit && isVisible(wavePlotter) && isVisible(specPlotter);
        else
            tf = audioBuffer.NumUnreadSamples > 0;
        end
    end
end