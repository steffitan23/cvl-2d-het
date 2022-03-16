# %%
# !pip install wandb
# !pip install tensorflow

# %%
import wandb
wandb.login()

# %%
import numpy as np
import tensorflow as tf
import tensorflow.keras.layers as layers
from tensorflow.keras.models import Model
from wandb.keras import WandbCallback

# %%
batch_size = 32
epochs = 10
validation_split = 0.05

x_train = np.load('training_dataset_x.npy', mmap_mode='r')[0:10000]
x_train = np.moveaxis(x_train, 1, -1)
y_train = np.load('training_dataset_y.npy', mmap_mode='r')[0:10000]
input_shape = x_train[0].shape

wandb.init(project="2d-het-project")


def encoder_decoder_model(input_shape, activation='relu', last_activation='relu',
                          encoder_filters=(32, 64, 128),
                          decoder_filters=(128, 64, 32),
                          output_dims=1):
    inp = layers.Input(input_shape)
    x = inp
    for filters in encoder_filters:
        x = layers.Conv2D(filters, (3, 3), padding='same')(x)
        x = layers.BatchNormalization()(x)
        x = layers.Activation(activation)(x)
        x = layers.MaxPooling2D((2, 2), padding='same')(x)
    x = layers.Conv2D(encoder_filters[-1] * 2, (3, 3), padding='same')(x)
    x = layers.BatchNormalization()(x)
    x = layers.Activation(activation)(x)
    for filters in decoder_filters:
        x = layers.UpSampling2D((2, 2))(x)
        x = layers.Conv2D(filters, (3, 3), padding='same')(x)
        x = layers.BatchNormalization()(x)
        x = layers.Activation(activation)(x)
    x = layers.Conv2D(output_dims, (3, 3), padding='same')(x)
    x = layers.BatchNormalization()(x)
    decoded = layers.Activation(last_activation)(x)

    model = Model(inputs=[inp], outputs=[decoded])
    return model


model = encoder_decoder_model(input_shape)
model.summary()
model.compile(optimizer='adam', loss='mse', metrics=['accuracy', 'mae'])

history = model.fit(x_train, y_train,
                    batch_size=batch_size,
                    epochs=epochs,
                    validation_split=validation_split,
                    callbacks=[WandbCallback()])

model.save('model.h5')

# Evaluate test data
# x_test = np.load('test_dataset_x.npy', mmap_mode='r')
# y_test = np.load('test_dataset_y.npy', mmap_mode='r')

# prediction = model.predict(x_test)

# compute error metrics etc ...




